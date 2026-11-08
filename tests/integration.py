"""Usage: python tests/integration.py path/to/hsms[.exe]. Standard library only."""
import os, pathlib, socket, struct, subprocess, sys, time, tempfile, random
ROOT=pathlib.Path(__file__).resolve().parents[1]
EXE=str(pathlib.Path(sys.argv[1]).resolve())
CASES=0

def check(value,label):
    global CASES
    if not value: raise RuntimeError(label)
    CASES+=1

def frame(stype=0,body=b'',stream=1):
    return struct.pack('>IHBBBBI',10+len(body),65535,stream,1,0,stype,0x12345678)+body

def run(data,env=None):
    return subprocess.run([EXE],input=data,capture_output=True,cwd=ROOT,timeout=10,env=env)

r=run(frame(1)+frame(5)+frame(9))
expected=bytearray(frame(2));expected[6:8]=b'\0\0'
expected2=bytearray(frame(6));expected2[6:8]=b'\0\0'
check(r.returncode==0 and r.stdout==expected+expected2,'Select/Linktest/Separate replies')
check(run(b'').returncode==0,'clean EOF')
for data in [b'\0',struct.pack('>I',9)+b'\0'*10,struct.pack('>I',0xffffffff)+b'\0'*10,frame(body=b'\x41\x05X'),frame(body=b'\xff\0'),frame(1,b'X'),frame(body=b'\x41\x03XYZ')[:-1]]:
    r=run(data);check(r.returncode==1 and b'HSMS error:' in r.stderr,'malformed frame rejected')
check(run(frame(body=b'\x41\x03ABC')+frame(9)).returncode==0,'valid data parsing')
with tempfile.TemporaryDirectory() as tmp:
    cfg=pathlib.Path(tmp)/'missing.ini'
    cfg.write_text('[RUNTIME]\nENABLE_MQ=Y\nTRANSACTION_DLL=./missing-transaction-module\n[MQHOST]\nCONF_MQ_MGR=QM\nCONNECTION=HOST\nRECEIVE_QUEUE=Q\n')
    env=os.environ.copy();env['HSMS_CONFIG']=str(cfg)
    r=run(b'',env);check(r.returncode==1 and b'MQ worker failed' in r.stderr,'missing DLL fail safely')
with socket.socket() as probe:
    probe.bind(('127.0.0.1',0));port=probe.getsockname()[1]
p=subprocess.Popen([EXE,'--listen',str(port)],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
try:
    c=None
    for _ in range(100):
        try:c=socket.create_connection(('127.0.0.1',port),timeout=.2);break
        except OSError:
            if p.poll() is not None:raise RuntimeError(p.stderr.read().decode())
            time.sleep(.02)
    if c is None:raise RuntimeError('TCP listener did not start')
    with c:
        c.settimeout(3);data=frame(1)
        for start,end in [(0,1),(1,4),(4,9),(9,14)]:c.sendall(data[start:end])
        received=b''
        while len(received)<14:
            chunk=c.recv(14-len(received))
            if not chunk:raise RuntimeError('TCP closed before reply')
            received+=chunk
        check(received==expected,'fragmented TCP Select.req')
        c.sendall(frame(body=b'\x41\x03ABC')+frame(9))
    out,err=p.communicate(timeout=5)
    check(p.returncode==0 and not out,'TCP clean exit')
finally:
    if p.poll() is None:p.kill();p.wait()
rng=random.Random(2017)
for _ in range(100):
    body=bytes(rng.randrange(256) for _ in range(rng.randrange(1,40)))
    r=run(frame(body=body));check(r.returncode in (0,1),'fuzz no sanitizer crash')
print(f'{CASES} integration checks passed')

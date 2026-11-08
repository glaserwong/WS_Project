# 程式分析與修正紀錄

日期：2026-11-06。依原 ZIP 靜態檢查與本機動態測試整理；不代表已找出全部 bug。

| 問題 | 原位置 | 修正與影響 |
|---|---|---|
| 兩個佇列共用 gsQueueBuf/glTopIndex/glEndIndex，互相覆寫、重複釋放 | TCSQueue.cpp | 改每個 instance 擁有 deque/vector；mutex 保護所有存取 |
| 佇列 Put/Peek 直接依外部長度 memcpy，缺少界線 | TCSQueue.cpp | 驗證長度、容量、滿佇列；失敗不消耗訊息，支援 FIFO 與實際訊息長度 |
| 建構設定失敗留下未初始化狀態，dynamic 模式實際不存資料 | TCSQueue.cpp | 使用明確預設容量、檢查範圍，以 vector 儲存實際長度，不再以 dynamic flag 關閉 Put |
| 重複覆寫 tRecvThreadID、join 未建立的 tSendThreadID、重複 join/cancel | HsmsSock.cpp | 改為主執行緒收 SECS、單一標準 C++ MQ worker；依 joinable 收尾，啟動/例外均管理生命週期 |
| pthread_cond_timedwait 未持有 mutex、斷線旗標資料競爭 | HsmsSock.cpp | 移除錯誤輪詢佇列與條件等待；停止旗標改 atomic |
| 未初始化指標在失敗路徑被 delete、dlsym 失敗仍呼叫 | HsmsSock.cpp / mbxmq.cpp | RAII、值初始化、呼叫前檢查函式指標，DLL handle 有明確壽命 |
| 封包長度小於 10 時 memcpy 超出 allocation；大長度無上限 | PassiveController | 驗證 10..16MiB、精確讀滿 header/body；錯誤退出 |
| 把 SType 的索引常數 9 當成任意控制比較；stdin EOF 不退出 | PassiveController | 明確識別 Separate.req=9、Select.req=1、Linktest.req=5；EOF 正常結束 |
| SECS body 從 offset 10 複製（應為完整 frame offset 14） | SecsDispatcher | 移除錯誤中間複製，直接把完整 frame 交給 parser |
| stream 使用 0x0F，截掉 stream 高 3 bits | SecsDispatcher / ReplyHSMS | 改為 0x7F，保留 7-bit stream |
| 對未對齊 long 指標解參照、LP64 讀超出 4-byte system bytes | SecsParser.cpp | 以固定寬度整數及逐 byte 大端序讀取，XML 保留完整 unsigned 32 位值 |
| I4/U4 使用 long（跨平台位寬不同）與 I64 printf 格式不相容 | SecsParser.cpp | 以 uint64_t 累積後按 type 寬度解碼，stream 格式化數字；支援 1/2/3 length bytes |
| 不支援 type 造成 secs_item_len=0，無窮迴圈；越界讀長度/內容 | SecsParser.cpp | 有界 parser，拒絕未知 type、零 length-byte count、截斷與不對齊數字 |
| list 計數錯誤、空 list 與深度未處理 | SecsParser.cpp | 遞迴驗證 item 數量，上限 64 層，拒絕尾隨資料 |
| ASCII strcpy 到固定 1024-byte buffer，XML escape 後更易溢位 | SecsParser.cpp | 使用 std::string，不用固定暫存陣列；XML 特殊字元轉義與字元檢查 |
| BOOLEAN 僅 -1 為真，B 類型以 signed char 輸出 | SecsParser.cpp | BOOLEAN 非零為真，binary 為 unsigned 數值 |
| MQDispatcher 未初始化 RecvMQMsg、strlen 讀無 NUL 資料、錯誤 memset sizeof(pointer) | HsmsSock.cpp | 移除錯誤 queue dispatcher，worker 使用有界 vector 與尾端 NUL，明確限制文字路徑 |
| CMemList destructor/Clear 重複釋放風險、accessor 空殼 | memlist.cpp | 清空後重設 root、正確逐節點釋放，補 Add/Delete/Top/Next/Member |
| mbr 負載在 x64 可能未對齊 | memlist.h | alignas(max_align_t)，禁止複製擁有資源的類別；外部來源需重新編譯 |
| SML SetString 配對錯誤的 delete、淺拷貝後名稱懸空 | smldoc.cpp | 配對 delete[]，移除未使用名稱 allocation，避免 dangling pointer |
| SML Build 長度多算 padding、使用 'A' 而不是 SECS 0x41、重建洩漏 | smldoc.cpp | 使用實際 item size+2，修正 type，釋放舊 body；此有限 builder 僅接受單 byte 長度 <=255 |
| if(doctype = NULL) 是賦值而不是比較 | xmldoc.cpp | 初始化 metadata、檢查 doctype 長度再複製 |
| GetConfig substring 配對、LF/CRLF 最後一字遺失、跨 section、無回傳、行緩衝溢位 | export.cpp | getline 精確 key/section、trim、處理末行無 newline、可配置檔案路徑；新增 Sized API |
| SendRecvHSMS tmpBuf[2] 無 NUL 且未檢查 strchr 結果 | export.cpp | 驗證輸入、sscanf stream/function，檢查範圍；仍只送出，不宣稱同步接收 |
| SetWaitBit 用加法，重複設定會翻轉溢位 | hsmsmsg.cpp | 使用 bitwise OR |
| OpenMQ 吞掉錯誤；MBX Recv/Send 用 Close 結果覆蓋 Get/Put 錯誤 | export.cpp / mbxmq.cpp | 保留原始操作失敗碼 |
| MQ descriptor 未初始化、correlation binary ID 當 %s | mbxmq.cpp | 零初始化、固定 128-byte memcpy；不把 binary ID 當字串 |
| Unix 絕對路徑、POSIX 執行緒/dlopen、Windows 文字模式轉譯 | 多處 | 本地 ini/log、std::thread、LoadLibrary/GetProcAddress 包裝、stdin/stdout binary、Winsock TCP |
| 未實作操作空殼回傳成功 | 多處 | 單純 int 操作空殼改回傳 -1，保留未完成範圍，不以假實作掩蓋 |

## 仍需後續處理

- 原 CHSMSMsg 多數查詢/設定與 timeout/時間介面、CXMLDoc Parse/Build、CSMLDoc 的多數 Parse/Get/Load、CMQMsg 交易辨識、全域/DB/Bank API 仍無完整實作。
- CMemList Make 的規格未知，仍回傳 nullptr。
- `GetConfig` 舊 ABI 沒有容量參數，契約為 caller 至少提供 1024 bytes；外部新程式應用 `GetConfigSized`。
- MBX reply queue 的舊 output API 沒有容量欄位，caller 必須提供至少 128 bytes；binary message ID 指標必須指向完整 128 bytes。
- MQ DLL 需遵守 timeout、buffer capacity、ABI/CRT 契約；本版無法驗證未知 DLL 是否正確。若 DLL 阻塞且不理會 timeout，join 仍會等待。
- 未實作完整 HSMS 協定狀態/計時、設備互通、真實 MQ/交易 DLL 測試。現場驗收前必須補齊。

## 已執行驗證

- Apple Clang C++14，主程式與回歸測試編譯/連結。
- AddressSanitizer + UndefinedBehaviorSanitizer 啟用。
- C++ regression：固定寬度、stream 127/system bytes、高長度 ASCII、XML escape、空/巢狀 list、有號數、浮點、BOOLEAN、無效封包、記憶體節點清理、SML 重複 build、wait bit、佇列隔離/滿載/越界與 10000 筆並行 FIFO、缺 DLL、配置讀取及容量不足。
- Python integration：113 檢查，含控制封包回覆、EOF/截斷/惡意長度、資料解析、缺 DLL、原生 TCP 分段與 100 個固定種子的隨機 SECS payload。
- **VS2026 v145 與 Windows 11 x64 實機：未執行。** 提供 Windows build-and-test.cmd 供目標環境驗證，不附無法驗證的 EXE。

## VS2026 版本追加複查

- SendRecvHSMS / ReplyHSMS：檢查 cout.write/flush 狀態，失敗回 -1。
- 傳送名稱必須在 m_sfname[32] 內有 NUL；手動限制 S/F 數字長度和範圍，不再讓 sscanf 對過大十進位數轉成 int。
- ReplyHSMS：Function 255 無法再加 1，明確拒絕，避免變成 Function 0。
- HSMS 長度編碼改為固定寬度位移，移除浮點 pow。
- CSMLDoc::SetString 暫存使用 unique_ptr，Add 配置失敗時仍自動釋放。
- 新增傳送邊界、輸出錯誤與回覆封包欄位回歸檢查。
- C++ 業務 DLL 需匹配 VS2026、x64、CRT 與本版標頭；Microsoft 工具集相容性不代表未知 DLL 的結構/契約自動相容。

## Windows 編譯回報修正：waitmsg.h

waitmsg.h 遺漏了 POSIX 型別移植，仍宣告 pthread_mutex_t / pthread_cond_t。MSVC 不提供這兩個型別，會造成缺少型別/宣告語法錯誤（常見為 C4430，並可能連帶 C2146）。已改用 std::mutex / std::condition_variable 並直接 include 對應標頭，不需要額外安裝 pthread。這是先前移植的遺漏。

CWaitMsg 原本只有宣告，沒有實作；這次修正標頭的 Windows 可編譯性，並未新增同步等待功能。若後續呼叫其建構子、Wait、Set 或 Signal，仍需提供實作，否則會有連結錯誤。若有自訂 DLL 使用 CWaitMsg，需用新版標頭重新編譯以匹配其類別配置。

## Windows 編譯回報修正：export.cpp 的 std::getline

export.cpp 使用 std::string 與 std::getline 卻未直接 include <string>，先前本機編譯透過其他標頭間接取得宣告，掩蓋了移植缺陷。MSVC 可因此出現 C2039（getline 不是 std 成員）與 C3861（找不到識別項）。已直接加入 <string>，並使用 <cstring> 與 <cstdarg>，讓此來源檔明確取得使用中的標準宣告。此修正不需變更 include/lib 路徑或新增程式庫。

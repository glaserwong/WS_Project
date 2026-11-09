# 保留 MBX 介面的 MSMQ 實作

本目錄以使用者提供的 `mbx.h`／IBM MQ 版 `mbx.cpp` 為依據，保留七個函式名稱、參數順序、C linkage、資料型別及 `MBXMD` 佈局，改用 Windows MSMQ API。原下載檔案未被修改；本目錄的 `mbx.h` 與提供的檔案相同。

產物為 **Windows x64 `mbx.dll`**，不需要 IBM MQ SDK。這是 MSMQ 後端，不會與既有 IBM MQ Queue Manager 通訊；IBM MQ 佇列內容也不會自動搬移。

## 函式對照

| 保留的函式 | MSMQ 行為 |
| --- | --- |
| `MBXCONNX(hmbm, mbm, cnn)` | 建立主機連線設定 context，回傳 32-bit 邏輯 handle；MSMQ 沒有 Queue Manager／channel，`mbm` 保留但不參與連線 |
| `MBXDISC(hmbm)` | 關閉 context 及其佇列邏輯 handle，成功後設為 0 |
| `MBXOPEN(hmbm, hmb, mb, mode)` | `MQOpenQueue`；支援接收、送出及瀏覽模式 |
| `MBXCLOSE(hmbm, hmb)` | 關閉邏輯 handle，成功後設為 0 |
| `MBXPUT(...)` | `MQSendMessage`，保留二進位 body 及 MBX metadata |
| `MBXPEEK(...)` | cursor FIRST／NEXT 瀏覽，不刪除訊息 |
| `MBXGET(...)` | 先瀏覽篩選，再使用 Lookup ID 取出符合的訊息，未符合者保留 |

MSMQ 原生 handle 是指標寬度，原介面的 handle 則是 32-bit `long`。實作使用內部 handle 表，不會將 64-bit 指標截斷為 32-bit。表格使用 Windows SRWLOCK 同步；本模組沒有建立 pthread 或 C++ 工作執行緒。

`MBXCONNX` 不會建立持續的網路 session，也不代表遠端主機／MSMQ 服務已驗證可用；實際錯誤可能在 OPEN 或 PUT／GET 出現。送往遠端的 MSMQ 訊息還涉及非同步投遞，PUT 成功不能等同遠端應用已接收。

## Visual Studio 2026 建置

1. 安裝「使用 C++ 的桌面開發」、MSVC v145 與 Windows SDK。
2. 開啟 `mbx_msmq.sln`，選擇 `Debug | x64` 或 `Release | x64`。
3. 建置方案，輸出位於 `bin\x64\<Configuration>\mbx.dll`、`mbx.lib`。

專案連結 Windows SDK 的 `mqrt.lib` 與 `ole32.lib`。可由專案屬性設定「C/C++ → 一般 → 其他 Include 目錄」、「連結器 → 一般 → 其他程式庫目錄」、「連結器 → 輸入 → 其他相依性」。本模組使用 C++17；與 HSMS 專案的 C11 執行緒模組各自獨立。

PowerShell 建置：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build-and-test.ps1 -Configuration Release
```

此命令只建置並檢查 DLL 為 x64 PE，不會連線或改動佇列。

## 啟用 MSMQ 與收送測試

Windows 必須提供並啟用「Message Queuing／訊息佇列」的 Server 功能，且 MSMQ 服務正在執行。可在「開啟或關閉 Windows 功能」設定；不需要 Active Directory 整合即可測試本機 private queue。確認使用者具有建立、讀取、寫入及刪除測試佇列的權限。

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build-and-test.ps1 -Configuration Release -RunIntegration
```

測試會建立唯一名稱的**本機、非交易式 private queue**，完成測試後刪除該測試佇列，不使用現場既有佇列。測試涵蓋二進位資料、大於初始緩衝區的訊息、ID／correlation metadata、回覆佇列、FIRST／NEXT、選擇性接收、截斷、有限等待、無訊息及失效 handle。

正式佇列需要由管理者預先建立；`MBXOPEN` 不會自動建立佇列。本版本使用 `MQ_NO_TRANSACTION`，支援**非交易式佇列**；不支援 MSMQ transaction／DTC。對交易式佇列操作時將由 MSMQ 回傳錯誤。

## 連線與佇列名稱規則

`cnn` 使用 MSMQ 電腦名稱，例如 `.`、`localhost`、`SERVER01`。為方便原介面遷移，`SERVER01(1414)` 會取 `SERVER01`，括號內的 IBM MQ port 不使用；沒有建立 TCP 1414 連線。

`mb` 接受 UTF-8 字串：

| 輸入 | 意義 |
| --- | --- |
| `SHARE.TCSI` | `cnn主機\private$\SHARE.TCSI` |
| `private$\SHARE.TCSI` | 同上 |
| `.\private$\SHARE.TCSI` | 指定本機 private queue |
| `SERVER01\private$\SHARE.TCSI` | 指定主機與佇列路徑 |
| `DIRECT=OS:SERVER01\private$\SHARE.TCSI` | 直接使用 MSMQ format name |
| `FORMATNAME:DIRECT=OS:SERVER01\private$\SHARE.TCSI` | 先移除 `FORMATNAME:` 前綴 |
| `PUBLIC=...`／`PRIVATE=...` | 使用既有 MSMQ format name |

format name 前綴使用上述大寫形式。遠端收取的支援、驗證與權限依 MSMQ format name、網域及主機設定而定；不能假定所有 DIRECT transport 都允許遠端讀取。應先通過本機測試，再另行測試遠端情境。

## 訊息欄位與相容性

- `buf/len` 是二進位資料，不呼叫 `strlen`，也不自動附加 NUL；請依 `md.len` 處理實際長度。
- MSMQ 原生 MessageId／CorrelationId 為 20 bytes；MBX 欄位為 128 bytes。以固定格式的 MSMQ **Extension** 儲存完整的 `rmbm/rmb/msgid1/msgid2/type/exp`，避免截掉邏輯識別碼。body 不包入額外 envelope。
- `msgid1` 全零時，PUT 後以 MSMQ 產生的 20-byte ID 回填，剩餘 bytes 為零。非零的自訂邏輯 ID 儲存在 Extension；不能改寫 MSMQ 原生 MessageId。
- `MBXMT_REQUEST` 每次產生新的 correlation，對應原始 IBM 程式的 NEW_CORREL_ID 意圖；其他訊息保留呼叫端提供的 correlation。PUT 成功後回填 `md.msgid1/msgid2/len`，方便後續查詢及回覆。
- GET／PEEK 的 `msgid1/msgid2` 指標為 NULL 或其 128 bytes 全零，表示不篩選；非零則完整比較 128 bytes，兩者都有值時必須同時符合。識別碼是 binary，不能只用 C 字串比較。
- `md.rmbm` 改解釋為回覆主機，`md.rmb` 為回覆佇列／format name；送出時也設定原生 MSMQ response queue。保留 `rmbm/rmb` 欄位不代表 IBM Queue Manager 名稱會自動轉成 MSMQ 主機。
- 對沒有 MBX Extension 的一般 MSMQ 訊息，讀取原生 20-byte IDs，其餘補零；AppSpecific 為 0、1、2 時作為訊息 type。原生 response queue 可放進 127 bytes UTF-8 時回填 `rmb`。
- 其他 MSMQ 應用若要識別 MBX 的完整 128-byte 邏輯 ID 或自訂 metadata，必須讀取 Extension，不能只讀原生 MSMQ ID 欄位。
- `md.exp < 0` 以 MSMQ 的 `LONG_LIVED`（約 68 年，作為實務上的無限期）設定；`0` 表示立即到期。MSMQ TTL 的單位是秒，正的毫秒數**向上取整**；例如 1501 ms 以 2 秒設定。MBX Extension 回傳原先要求的 exp，不代表剩餘生存時間；一般原生訊息的 lifetime 則換算成毫秒。
- 所有 metadata 必須先初始化。建議 `MBXMD md = {}; md.exp = -1;`；`rmbm/rmb` 必須是欄位內有 NUL 結尾的 UTF-8 字串。

範例（省略錯誤檢查，正式程式應檢查每個回傳值）：

```cpp
MBXHMBX connection = 0;
MBXHMB queue = 0;
char host[] = ".";
char manager[] = ""; // MSMQ 不使用此欄位
char name[] = "SHARE.TCSI";
char payload[] = "hello MSMQ";
MBXMD md = {};
md.exp = -1;
MBXCONNX(&connection, manager, host);
MBXOPEN(connection, &queue, name, MBXOO_OUTPUT);
MBXPUT(connection, queue, payload, sizeof(payload) - 1, &md);
MBXCLOSE(connection, &queue);
MBXDISC(&connection);
```

## 回傳碼與等待

| 回傳碼 | 意義 |
| --- | --- |
| `0` | 成功 |
| `1` / `MBXRC_NO_MSG_AVAILABLE` | 等待期間沒有符合條件的訊息 |
| `2` / `MBXRC_TRUNCATED_MSG_ACCEPTED` | 只複製 caller buffer 可容納的前段；`md.len` 為完整 body 長度 |
| 負值 HRESULT | MSMQ／Windows 錯誤，可用十六進位 HRESULT 查詢；不是 IBM MQ reason code |
| `-10001` | 一般 MSMQ 訊息的 response queue 太長，無法無損存入 MBX metadata；不取走訊息 |
| `-10002` | 尚未 FIRST 就執行 NEXT |

`timer < 0` 無限等待、`0` 不等待、正值為毫秒。篩選時會瀏覽未符合的訊息，不將它們取走。GET 以 Lookup ID 取走已符合的訊息；若另一消費者先取走，會繼續尋找。

**截斷的 GET 仍會取走整筆訊息**，符合原 `MQGMO_ACCEPT_TRUNCATED_MSG` 行為；截斷的 PEEK 不取走訊息。不要在不能接受資料損失的呼叫路徑中忽略回傳碼 2。

同一個 browse handle 的 FIRST／NEXT 共用 cursor，應交由單一瀏覽流程使用。不同 handle 可以並行操作。CLOSE／DISC 會將邏輯 handle 標為失效；同步等待以最長約 100 ms 的原生等待片段檢查關閉狀態，系統排程／API 延遲不在此限。已開始的操作仍可能完成；這不是交易取消保證。

## 與現有 HSMS 專案搭配

選擇相同 Debug／Release 組態建置，將 `mbx.dll` 放到 `hsms.exe` 所在資料夾；現有 `MBXMQ` 動態載入的 MBX 函式介面可沿用。`mcsstdfunc.dll` 是另一個交易模組，本專案不取代它。

此目錄提供獨立方案，沒有修改原 HSMS 的佇列設定或自動覆蓋其執行檔目錄。現有 HSMS 的 `HsmsSock.cpp` 還有寫死的 MQ 位址／佇列名稱，需改為實際 MSMQ 主機與 private queue；其中 `MBXMQ` 包裝層的 metadata 初始化與錯誤碼傳遞問題也應修正後再做整合，不能僅憑放入 DLL 就判定完整交易已可用。

## 驗證狀態

目前環境是 macOS，沒有 Windows SDK／MSMQ，**尚未在 Visual Studio 2026 編譯或在 Windows MSMQ 服務執行此版本**。已檢查原始 header 一致性、七個匯出函式介面及專案來源參考；Windows 建置與實際收送測試仍需執行上面的腳本。未以模擬 Windows 標頭或假佇列宣稱實測通過。

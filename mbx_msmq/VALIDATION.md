# 驗證紀錄

2026-11-08，Asia/Taipei。

已執行靜態檢查：

- `mbx.h` 與使用者提供的下載檔案逐位元組相同。
- 七個 MBX 函式的名稱、參數型別、參數名稱及順序與原 header 一致。
- DLL `.def` 匯出七個未修飾的 C 名稱。
- 兩個 Visual Studio 專案的 XML、來源檔案及 project reference 均存在，設定為 v145 / Debug、Release / x64。
- 沒有依賴 IBM MQ 的 `cmqc.h`、`cmqxc.h`、MQCONNX/MQGET/MQPUT 或 pthread。

目前執行環境為 macOS，沒有 Windows SDK、Visual Studio 或 MSMQ。**本次未實際編譯 DLL，也未執行 MSMQ 收送測試。** 原生 API 的編譯及 runtime 行為仍需在 Windows 確認，靜態檢查不代表這些測試已通過。

請在 Windows 執行：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build-and-test.ps1 -Configuration Debug -RunIntegration
powershell -NoProfile -ExecutionPolicy Bypass -File .\build-and-test.ps1 -Configuration Release -RunIntegration
```

`msmq_integration.exe` 使用唯一名稱的本機非交易式佇列，測試二進位收送、8192-byte body、自訂／產生的 IDs、回覆 metadata、瀏覽、選擇性接收且保留未符合訊息、截斷接收、逾時、失效 handle、一般原生 MSMQ 訊息相容性。測試尚未執行；測試不涵蓋遠端 ACL、遠端讀取限制、交易式佇列或完整 HSMS 業務交易。

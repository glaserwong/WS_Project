# HSMS — Visual Studio 2026 / Windows 11 x64 移植版

本版以先前 VS2017 移植結果為基礎，保留其 bug 修正，另建 VS2026 專案。

本版依原 `mak/hsms.mak` 的實際主程式來源移植，為 C++ 主程式（不是純 C）。
提供原生 MSBuild `.sln` / `.vcxproj`，平台工具集 v145，C++14，Debug/Release x64。
原始 ZIP 未修改。本交付壓縮檔只含移植所需原始碼、設定、專案、測試及文件，不含 AIX/Linux 執行檔或 `.a/.so/.o`。

## 驗證狀態

- macOS Apple Clang：主程式與測試程式皆已編譯、連結及執行。
- C++ 回歸測試與 113 項整合檢查已通過，使用 AddressSanitizer 和 UndefinedBehaviorSanitizer，未見診斷。
- **未在 Microsoft VS2026 v145 編譯器或 Windows 11 實機執行；本環境沒有該工具鏈。不能將上述結果視為 Windows 實測通過。**
- `windows/build-and-test.cmd` 可在目標機重建兩種組態並執行測試。交付中未附 Windows EXE。

## VS2026 開啟與編譯

1. 安裝 Visual Studio 2026 18.x，勾選「使用 C++ 的桌面開發」、VC++ v145 x64/x86 工具與 Windows 10 SDK。
2. 解壓縮後開啟 `hsms.sln`，選擇 `Debug | x64` 或 `Release | x64`。
3. 若 SDK 版本無法解析，右鍵方案 →「重定目標方案」，選擇已安裝的 Windows 10 SDK，平台工具集仍選 v145。
4. 建置 →「重建方案」。`hsms` 是主程式；`hsms_tests` 是回歸測試程式。
5. 輸出位置：`bin\x64\Debug\` 或 `bin\x64\Release\`。設定檔會複製到輸出目錄下 `ini\hsms.ini`。
6. 可在「x64 Native Tools Command Prompt for VS 2026」執行 `windows\build-and-test.cmd`。

Release 使用 `/MD`，部署 Windows 11 x64 時需安裝 Microsoft 最新 Visual C++ v14 Redistributable x64（版本須不低於建置使用的 MSVC runtime）。
Debug 使用 `/MDd`，請在開發環境執行。這是主控台程式，沒有新增應用程式 GUI；本需求中的 UI 設定是 Visual Studio 專案屬性視窗。

## 在 Visual Studio UI 設定 include / lib

右鍵 `hsms` 專案 →「屬性」，選「所有組態」與「x64」：

| 要設定的項目 | 屬性頁路徑 | 範例 |
|---|---|---|
| include 路徑 | 組態屬性 → C/C++ → 一般 → 其他 Include 目錄 | `D:\SDK\include;%(AdditionalIncludeDirectories)` |
| lib 搜尋路徑 | 組態屬性 → 連結器 → 一般 → 其他程式庫目錄 | `D:\SDK\lib\x64;%(AdditionalLibraryDirectories)` |
| 要連結的 lib 名稱 | 組態屬性 → 連結器 → 輸入 → 其他相依性 | `Vendor.lib;%(AdditionalDependencies)` |

保留「繼承自父系或專案預設值」；基礎 include 路徑、Ws2_32.lib 已設定。
如需讓兩個專案共用設定：檢視 → 其他視窗 → 屬性管理員，開啟 `UserPaths` 屬性表，於「使用者巨集」設定：

- `HsmsExtraInclude`：額外 include 目錄，多個以分號分隔。
- `HsmsExtraLib`：額外 x64 lib 目錄。
- `HsmsExtraDependencies`：額外 `.lib` 名稱。

預設主程式不需要外部 `.lib`。改 lib 路徑不會把 AIX `.a` 或 `.so` 轉成 Windows 格式；第三方元件必須是 Windows x64。

## 執行方式

從輸出目錄執行：

```bat
cd bin\x64\Release
hsms_tests.exe
hsms.exe --listen 5000
```

`--listen 5000` 在所有 IPv4 介面監聽 TCP 5000，接受一條連線；該連線 EOF 或 Separate.req 後程式結束。Windows 防火牆依使用環境允許此程式或連接埠。啟動訊息與錯誤寫到 stderr，資料封包不混入文字。紀錄檔為工作目錄下 `hsms.log`。

沒有參數時，保留原主程式的二進位 stdin/stdout 模式，適合管線或既有服務包裝器；不能在主控台輸入文字模擬二進位 HSMS 封包。

在 VS 的專案屬性 → 偵錯：命令引數填 `--listen 5000`，工作目錄填 `$(OutDir)`。

可設定環境變數 `HSMS_CONFIG` 指向其他 ini 檔；未設定時讀取相對工作目錄的 `ini/hsms.ini`。
每次重建會從專案 ini 覆寫輸出 ini，持久設定請修改專案 ini 或使用 `HSMS_CONFIG`。

如已安裝 Python 3，可選擇執行額外整合測試（不是執行主程式的必要相依）：

```bat
python tests\integration.py bin\x64\Release\hsms.exe
```

## 缺少的外部元件與功能界線

依您確認，先完成可建置版本，因此預設 `ENABLE_MQ=N`。

- MQ 需要供應商提供 Windows x64 `mbx.dll`，含 `MBXCONNX/MBXDISC/MBXOPEN/MBXCLOSE/MBXGET/MBXPUT`；目前沒有這個可用 DLL。
- MQ 交易還需要 `mcsstdfunc.dll` 與 `tx_S01F01_QSTD` 匯出。原 ZIP 沒有這個元件或其原始碼。
- 取得相符元件後，放在程式工作目錄，設定 ini 的 `TRANSACTION_DLL`、MQ manager/connection/queue，再將 `ENABLE_MQ=Y`。應先向供應商確認呼叫慣例、結構配置與 `/MD` CRT，並以本版標頭重新建置 C++ 交易元件。
- MBX 的 long 欄位明確定義為 32 位整數，適用 Windows LLP64。CMQMsg 等包含 C++ 類別，不能與 AIX 二進位或不相符的 C++ ABI 混用。
- MQ wrapper 缺 DLL/匯出或回傳錯誤時不再呼叫空指標、不再吞掉原始錯誤。未進行真實 MQ 往返驗證。
- MQ 目前只供文字訊息交易。呼叫端尾端補 NUL；內嵌 NUL 的二進位 MQ 訊息不可透過本條路徑傳送。MBX API 的實際長度契約仍需供應商確認。
- 原本 SECS 業務 callback 就被註解，本版不虛構業務回覆。SECS 資料訊息解析後寫入 log；控制封包可回覆 Select.req/Linktest.req。
- 這不是完整 HSMS/SEMI E37 實作：不含完整連線狀態機、主動連線、自動重連、T3/T5/T6/T7/T8 計時與多連線伺服器；TCP 接收仍可能等候對方資料，關閉服務可由作業系統終止程序。
- JIS/非 ASCII 文字需指定編碼政策後再支援，目前明確拒絕，避免產生不合法 XML。
- 原始 SML/XML/訊息/DB/全域資料函式中有大量空殼。本版未將其補成完整產品；多數未實作且以 int 表示操作結果的空殼改回傳 -1，避免誤報成功。其他查詢/void 空殼仍需依業務定義實作。完整內容見 BUGFIX_REPORT.md。

原 `com/comtrx` C 程式、`com/domext` 和測試範例不在 `hsms.mak` 主程式來源清單內（domext 原以預編譯庫連結，但本主程式連結不需要其符號），故不放入移植專案。它們仍在原 ZIP 中；本交付並未宣稱這些其他元件也完成移植。

## 工具集依據

使用 VS2026 的 v145 工具集與 Solution version 18。需安裝「使用 C++ 的桌面開發」、MSVC v145 x64/x86 工具與 Windows SDK；不必安裝舊 v141。
Microsoft 官方相容性資料：https://learn.microsoft.com/en-us/cpp/porting/binary-compat-2015-2017

2026-11-06 複查另修正：封包傳送輸出失敗仍回成功、固定名稱非 NUL 結尾造成越界讀取、回覆 Function 255 溢位，以及 SML 配置失敗時暫存記憶體洩漏。參見 BUGFIX_REPORT.md。

## waitmsg.h 編譯修補

若舊下載版在 waitmsg.h 的 pthread_mutex_t / pthread_cond_t 行出現缺少型別錯誤，請使用更新後的完整專案包，或將兩個欄位分別改為 std::mutex / std::condition_variable，並 include <mutex> 與 <condition_variable>。不必新增 pthread 的 include/lib 路徑。CWaitMsg 原始方法仍只有宣告，不能視為已實作的等待服務。

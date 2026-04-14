#include "pch.h"
#include "framework.h"
#include "MfcProgram.h"
#include "MfcProgramDlg.h"
#include "afxdialogex.h"

/*
    ------------------------------------------------------------------------
    프로젝트 내부 헤더
    ------------------------------------------------------------------------
    - ImageRepository : MySQL CRUD 담당
    - ImageApiService : cv api C++ SDK 호출 담당
    - TempImageUtil   : DB에서 읽은 BLOB 이미지를 임시 파일로 저장
    - StringUtil      : CString <-> std::string 변환
*/
#include "ImageRepository.h"
#include "ImageApiService.h"
#include "TempImageUtil.h"
#include "StringUtil.h"

/*
    ------------------------------------------------------------------------
    ATL / Win32 관련 헤더
    ------------------------------------------------------------------------
    - CImage      : 이미지 파일 로드 및 Draw 용도
    - Shlwapi     : PathFileExists 사용
*/
#include <atlimage.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*
    ------------------------------------------------------------------------
    정보 대화 상자
    ------------------------------------------------------------------------
    MFC 기본 템플릿이 생성해주는 About 박스용 클래스
    현재 핵심 기능과 직접 관련은 없지만 기본 구조 유지
*/
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

/*
    ========================================================================
    CMfcProgramDlg
    ========================================================================
*/

/*
    ------------------------------------------------------------------------
    생성자
    ------------------------------------------------------------------------
    다이얼로그 생성 시 아이콘 로드
*/
CMfcProgramDlg::CMfcProgramDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_MFCPROGRAM_DIALOG, pParent)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/*
    ------------------------------------------------------------------------
    DoDataExchange
    ------------------------------------------------------------------------
    리소스 편집기에서 배치한 컨트롤과
    C++ 멤버 변수를 연결하는 함수

    여기서 연결하지 않으면:
    - _listResults.SetExtendedStyle(...)
    - _editTitle.GetWindowTextW(...)
    - _staticStatus.SetWindowTextW(...)
    같은 코드가 동작하지 않음
*/
void CMfcProgramDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    // 결과 목록 리스트 컨트롤
    DDX_Control(pDX, IDC_LIST_RESULTS, _listResults);

    // 입력창
    DDX_Control(pDX, IDC_EDIT_TITLE, _editTitle);
    DDX_Control(pDX, IDC_EDIT_IMAGE_URL, _editImageUrl);
    DDX_Control(pDX, IDC_EDIT_BLUR_KSIZE, _editBlurKsize);
    DDX_Control(pDX, IDC_EDIT_WIDTH, _editWidth);
    DDX_Control(pDX, IDC_EDIT_HEIGHT, _editHeight);

    // 상태 표시 텍스트 및 결과 이미지 표시 영역
    DDX_Control(pDX, IDC_STATIC_STATUS, _staticStatus);
    DDX_Control(pDX, IDC_STATIC_RESULT_IMAGE, _staticResultImage);
}

/*
    ------------------------------------------------------------------------
    메시지 맵
    ------------------------------------------------------------------------
    버튼 클릭 / 리스트 선택 변화 / 그리기 관련 메시지를
    어떤 멤버 함수가 처리할지 연결
*/
BEGIN_MESSAGE_MAP(CMfcProgramDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()

    ON_BN_CLICKED(IDC_BUTTON_PROCESS, &CMfcProgramDlg::OnBnClickedButtonProcess)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CMfcProgramDlg::OnBnClickedButtonSave)
    ON_BN_CLICKED(IDC_BUTTON_DELETE, &CMfcProgramDlg::OnBnClickedButtonDelete)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CMfcProgramDlg::OnBnClickedButtonRefresh)

    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_RESULTS, &CMfcProgramDlg::OnLvnItemchangedListResults)
END_MESSAGE_MAP()

/*
    ------------------------------------------------------------------------
    OnInitDialog
    ------------------------------------------------------------------------
    대화 상자 생성 직후 한 번 호출됨

    여기서 하는 일:
    1. 기본 아이콘 세팅
    2. 리스트 컨트롤 컬럼 초기화
    3. 입력 기본값 세팅
    4. 상태/결과 이미지 초기화
    5. DB 연결
    6. 테이블 생성 보장
    7. 저장된 목록 조회
*/
BOOL CMfcProgramDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* menu = GetSystemMenu(FALSE);
    if (menu != nullptr)
    {
        CString aboutMenuString;
        BOOL isStringLoaded = aboutMenuString.LoadString(IDS_ABOUTBOX);
        ASSERT(isStringLoaded);

        if (!aboutMenuString.IsEmpty())
        {
            menu->AppendMenu(MF_SEPARATOR);
            menu->AppendMenu(MF_STRING, IDM_ABOUTBOX, aboutMenuString);
        }
    }

    SetIcon(_hIcon, TRUE);
    SetIcon(_hIcon, FALSE);

    InitializeListControl();
    InitializeDefaultValues();
    SetStatusText(L"상태: 준비");
    ClearResultImage();

    CString debugInfo;
    debugInfo.Format(
        L"[DB CONFIG] host=%S, port=%d, user=%S, database=%S\n",
        _dbConfig.host.c_str(),
        _dbConfig.port,
        _dbConfig.user.c_str(),
        _dbConfig.database.c_str()
    );
    OutputDebugString(debugInfo);

    std::string error;

    if (!_repository.Connect(_dbConfig, error))
    {
        CString message;
        message.Format(L"DB 연결 실패:\n%S", error.c_str());
        AfxMessageBox(message);
        SetStatusText(L"상태: DB 연결 실패");
        return TRUE;
    }

    if (!_repository.EnsureTable(error))
    {
        CString message;
        message.Format(L"테이블 생성 실패:\n%S", error.c_str());
        AfxMessageBox(message);
        SetStatusText(L"상태: 테이블 생성 실패");
        return TRUE;
    }

    ReloadProcessedImageList();
    SetStatusText(L"상태: DB 연결 완료");

    return TRUE;
}

/*
    ------------------------------------------------------------------------
    InitializeListControl
    ------------------------------------------------------------------------
    결과 목록 List Control을 Report View 형식으로 사용하기 위한 초기화

    표시 컬럼:
    - ID
    - 제목
    - 처리 옵션
    - 저장일
*/
void CMfcProgramDlg::InitializeListControl()
{
    // 전체 행 선택 + 격자선 표시
    _listResults.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // 컬럼 생성
    _listResults.InsertColumn(0, L"ID", LVCFMT_LEFT, 60);
    _listResults.InsertColumn(1, L"제목", LVCFMT_LEFT, 140);
    _listResults.InsertColumn(2, L"처리 옵션", LVCFMT_LEFT, 220);
    _listResults.InsertColumn(3, L"저장일", LVCFMT_LEFT, 150);
}

/*
    ------------------------------------------------------------------------
    InitializeDefaultValues
    ------------------------------------------------------------------------
    사용자가 아무 값도 안 넣어도 테스트가 가능하도록
    blur / resize 기본값을 미리 채워둔다.
*/
void CMfcProgramDlg::InitializeDefaultValues()
{
    _editBlurKsize.SetWindowTextW(L"9");
    _editWidth.SetWindowTextW(L"512");
    _editHeight.SetWindowTextW(L"512");
}

/*
    ------------------------------------------------------------------------
    SetStatusText
    ------------------------------------------------------------------------
    상태 표시용 Static Text에 메시지를 출력
*/
void CMfcProgramDlg::SetStatusText(const CString& text)
{
    _staticStatus.SetWindowTextW(text);
}

/*
    ------------------------------------------------------------------------
    GetEditText
    ------------------------------------------------------------------------
    CEdit 컨트롤에서 현재 입력 문자열을 읽어서 반환
*/
CString CMfcProgramDlg::GetEditText(const CEdit& edit) const
{
    CString text;
    const_cast<CEdit&>(edit).GetWindowTextW(text);
    return text;
}

/*
    ------------------------------------------------------------------------
    IsChecked
    ------------------------------------------------------------------------
    체크박스 컨트롤 ID를 받아 현재 체크 상태를 bool로 반환
*/
bool CMfcProgramDlg::IsChecked(int controlId) const
{
    return IsDlgButtonChecked(controlId) == BST_CHECKED;
}

/*
    ------------------------------------------------------------------------
    BuildOperationSummaryFromUi
    ------------------------------------------------------------------------
    현재 UI에서 체크한 옵션을 사람이 읽기 쉬운 문자열로 요약

    예:
    - grayscale
    - blur(ksize=9)
    - resize(512x512)

    용도:
    - DB 저장용 operation_summary
    - 화면 표시용 설명
*/
CString CMfcProgramDlg::BuildOperationSummaryFromUi() const
{
    CString summary;

    if (IsChecked(IDC_CHECK_GRAYSCALE))
    {
        if (!summary.IsEmpty())
            summary += L", ";

        summary += L"grayscale";
    }

    if (IsChecked(IDC_CHECK_BLUR))
    {
        CString blurKsize = GetEditText(_editBlurKsize);

        if (!summary.IsEmpty())
            summary += L", ";

        summary += L"blur(ksize=" + blurKsize + L")";
    }

    if (IsChecked(IDC_CHECK_RESIZE))
    {
        CString width = GetEditText(_editWidth);
        CString height = GetEditText(_editHeight);

        if (!summary.IsEmpty())
            summary += L", ";

        summary += L"resize(" + width + L"x" + height + L")";
    }

    if (summary.IsEmpty())
    {
        summary = L"(선택된 옵션 없음)";
    }

    return summary;
}

/*
    ------------------------------------------------------------------------
    ReloadProcessedImageList
    ------------------------------------------------------------------------
    DB에서 저장된 processed_images 목록을 다시 읽어
    List Control에 표시

    동작:
    1. 기존 리스트 내용 삭제
    2. DB에서 전체 목록 조회
    3. 각 레코드를 리스트에 한 줄씩 삽입
*/
void CMfcProgramDlg::ReloadProcessedImageList()
{
    _listResults.DeleteAllItems();

    std::vector<ProcessedImageRecord> records;
    std::string error;

    if (!_repository.GetAllProcessedImages(records, error))
    {
        SetStatusText(L"상태: 리스트 조회 실패");
        return;
    }

    for (int i = 0; i < static_cast<int>(records.size()); ++i)
    {
        const ProcessedImageRecord& record = records[i];

        CString idText;
        idText.Format(L"%d", record.id);

        _listResults.InsertItem(i, idText);
        _listResults.SetItemText(i, 1, StringUtil::Utf8ToCString(record.title));
        _listResults.SetItemText(i, 2, StringUtil::Utf8ToCString(record.operationSummary));
        _listResults.SetItemText(i, 3, StringUtil::Utf8ToCString(record.createdAt));
    }
}

/*
    ------------------------------------------------------------------------
    GetSelectedRecordId
    ------------------------------------------------------------------------
    현재 선택된 리스트 항목의 0번 컬럼(ID)을 읽어서 반환

    선택된 항목이 없으면 0 반환
*/
int CMfcProgramDlg::GetSelectedRecordId() const
{
    POSITION position = _listResults.GetFirstSelectedItemPosition();

    if (position == nullptr)
    {
        return 0;
    }

    int selectedIndex = _listResults.GetNextSelectedItem(position);
    CString idText = _listResults.GetItemText(selectedIndex, 0);

    return _wtoi(idText);
}

/*
    ------------------------------------------------------------------------
    ClearResultImage
    ------------------------------------------------------------------------
    결과 이미지 Picture Control에 연결된 비트맵을 제거하고 화면 초기화

    주의:
    - 이전에 SetBitmap으로 연결한 HBITMAP은 직접 DeleteObject 해줘야 함
    - 안 지우면 비트맵 리소스 누수 가능
*/
void CMfcProgramDlg::ClearResultImage()
{
    _staticResultImage.SetBitmap(nullptr);

    if (_hResultBitmap != nullptr)
    {
        ::DeleteObject(_hResultBitmap);
        _hResultBitmap = nullptr;
    }

    _staticResultImage.Invalidate();
    _staticResultImage.UpdateWindow();
}

/*
    ------------------------------------------------------------------------
    ShowImageInControl
    ------------------------------------------------------------------------
    이미지 파일을 CImage로 로드한 뒤 Picture Control에 맞춰 표시

    imagePath:
    - 표시할 이미지 파일 경로

    targetControl:
    - 실제 이미지를 보여줄 Picture Control

    bitmapHandle:
    - 이전 비트맵 해제 / 새 비트맵 연결을 위한 핸들 저장용 참조

    동작 과정:
    1. 파일 존재 여부 확인
    2. CImage로 이미지 로드
    3. Picture Control 크기 확인
    4. 메모리 DC에 비율 유지하며 이미지 그림
    5. HBITMAP을 Picture Control에 연결
*/
void CMfcProgramDlg::ShowImageInControl(const CString& imagePath, CStatic& targetControl, HBITMAP& bitmapHandle)
{
    if (::PathFileExists(imagePath) == FALSE)
    {
        AfxMessageBox(L"이미지 파일을 찾을 수 없음.");
        return;
    }

    CImage image;
    HRESULT hr = image.Load(imagePath);

    if (FAILED(hr))
    {
        AfxMessageBox(L"이미지 로드 실패.");
        return;
    }

    // Picture Control 내부 크기 얻기
    CRect rect;
    targetControl.GetClientRect(&rect);

    int controlWidth = rect.Width();
    int controlHeight = rect.Height();

    if (controlWidth <= 0 || controlHeight <= 0)
    {
        AfxMessageBox(L"이미지 표시 영역 크기가 올바르지 않음.");
        return;
    }

    // 컨트롤 DC 얻기
    CDC* dc = targetControl.GetDC();
    if (dc == nullptr)
    {
        AfxMessageBox(L"디바이스 컨텍스트 획득 실패.");
        return;
    }

    // 메모리 DC 생성
    CDC memDC;
    memDC.CreateCompatibleDC(dc);

    // Picture Control 크기와 같은 비트맵 생성
    CBitmap bitmap;
    bitmap.CreateCompatibleBitmap(dc, controlWidth, controlHeight);

    CBitmap* oldBitmap = memDC.SelectObject(&bitmap);

    // 흰 배경 채우기
    memDC.FillSolidRect(0, 0, controlWidth, controlHeight, RGB(255, 255, 255));

    int imageWidth = image.GetWidth();
    int imageHeight = image.GetHeight();

    if (imageWidth <= 0 || imageHeight <= 0)
    {
        memDC.SelectObject(oldBitmap);
        targetControl.ReleaseDC(dc);
        AfxMessageBox(L"이미지 크기가 올바르지 않음.");
        return;
    }

    /*
        --------------------------------------------------------------------
        비율 유지 리사이즈 계산
        --------------------------------------------------------------------
        Picture Control 안에 이미지를 넣되,
        찌그러지지 않도록 가로/세로 비율 유지
    */
    double scaleX = static_cast<double>(controlWidth) / static_cast<double>(imageWidth);
    double scaleY = static_cast<double>(controlHeight) / static_cast<double>(imageHeight);
    double scale = min(scaleX, scaleY);

    int drawWidth = static_cast<int>(imageWidth * scale);
    int drawHeight = static_cast<int>(imageHeight * scale);

    // 가운데 정렬을 위한 오프셋
    int offsetX = (controlWidth - drawWidth) / 2;
    int offsetY = (controlHeight - drawHeight) / 2;

    // 확대/축소 품질 향상
    ::SetStretchBltMode(memDC.GetSafeHdc(), HALFTONE);

    // 실제 이미지 그리기
    image.Draw(memDC.GetSafeHdc(), offsetX, offsetY, drawWidth, drawHeight);

    // 이전 비트맵 해제
    if (bitmapHandle != nullptr)
    {
        ::DeleteObject(bitmapHandle);
        bitmapHandle = nullptr;
    }

    // 새 비트맵을 Picture Control에 연결하기 위해 HBITMAP 분리
    bitmapHandle = (HBITMAP)bitmap.Detach();

    targetControl.SetBitmap(bitmapHandle);

    // DC 정리
    memDC.SelectObject(oldBitmap);
    targetControl.ReleaseDC(dc);
}

/*
    ------------------------------------------------------------------------
    OnBnClickedButtonProcess
    ------------------------------------------------------------------------
    [처리] 버튼 클릭 시 동작

    흐름:
    1. 제목 / URL 입력값 확인
    2. 체크된 처리 옵션 읽기
    3. cv api C++ SDK를 통해 서버 요청
    4. 결과 이미지 바이트를 _pendingImageBytes 에 저장
    5. DB에 바로 넣지 않고 "저장 가능 상태"로 유지
    6. 임시 파일로 저장해 미리보기 표시

    즉:
    - 처리 = 서버 호출 + 결과 받기
    - 저장 = DB insert
    으로 분리한 구조
*/
void CMfcProgramDlg::OnBnClickedButtonProcess()
{
    CString title = GetEditText(_editTitle);
    CString imageUrl = GetEditText(_editImageUrl);

    if (title.IsEmpty())
    {
        AfxMessageBox(L"제목을 입력하세요.");
        return;
    }

    if (imageUrl.IsEmpty())
    {
        AfxMessageBox(L"이미지 URL을 입력하세요.");
        return;
    }

    // 현재 UI 상태를 ProcessOptions 구조체로 정리
    ProcessOptions options;
    options.grayscale = IsChecked(IDC_CHECK_GRAYSCALE);
    options.blur = IsChecked(IDC_CHECK_BLUR);
    options.resize = IsChecked(IDC_CHECK_RESIZE);

    options.blurKsize = _wtoi(GetEditText(_editBlurKsize));
    options.width = _wtoi(GetEditText(_editWidth));
    options.height = _wtoi(GetEditText(_editHeight));

    // 현재 UI에는 output format 선택이 없으므로 png 고정
    options.outputFormat = "png";

    CString operationSummary;
    CString outputFormat;
    CString errorMessage;
    std::vector<unsigned char> imageBytes;

    bool ok = _apiService.ProcessImageFromUrl(
        _apiBaseUrl,
        imageUrl,
        options,
        imageBytes,
        operationSummary,
        outputFormat,
        errorMessage
    );

    if (!ok)
    {
        AfxMessageBox(L"처리 실패: " + errorMessage);
        SetStatusText(L"상태: 처리 실패");
        return;
    }

    /*
        --------------------------------------------------------------------
        처리 성공 결과를 pending 상태로 보관
        --------------------------------------------------------------------
        사용자가 [저장] 버튼을 누르면 이 데이터를 DB에 insert 하게 됨
    */
    _pendingImageBytes = imageBytes;
    _pendingOperationSummary = operationSummary;
    _pendingOutputFormat = outputFormat;
    _pendingTitle = title;
    _pendingImageUrl = imageUrl;
    _hasPendingResult = true;

    // 미리보기용 temp 파일 생성 후 결과 이미지 표시
    CString tempPath;
    if (TempImageUtil::SaveBytesToTempFile(_pendingImageBytes, _pendingOutputFormat, tempPath))
    {
        ShowImageInControl(tempPath, _staticResultImage, _hResultBitmap);
    }

    SetStatusText(L"상태: 처리 성공, 저장 가능");
    AfxMessageBox(L"처리 성공. 저장 버튼으로 DB에 저장 가능.");
}

/*
    ------------------------------------------------------------------------
    OnBnClickedButtonSave
    ------------------------------------------------------------------------
    [저장] 버튼 클릭 시 동작

    흐름:
    1. pending 결과가 있는지 확인
    2. ProcessedImageRecord 구성
    3. Repository를 통해 MySQL LONGBLOB 저장
    4. 리스트 새로고침
    5. pending 상태 해제

    주의:
    - 처리 버튼을 먼저 누르지 않으면 저장 불가
*/
void CMfcProgramDlg::OnBnClickedButtonSave()
{
    if (!_hasPendingResult || _pendingImageBytes.empty())
    {
        AfxMessageBox(L"먼저 처리 버튼을 눌러 결과를 생성하세요.");
        return;
    }

    ProcessedImageRecord record;
    record.title = StringUtil::CStringToUtf8(_pendingTitle);
    record.imageUrl = StringUtil::CStringToUtf8(_pendingImageUrl);
    record.operationSummary = StringUtil::CStringToUtf8(_pendingOperationSummary);
    record.outputFormat = StringUtil::CStringToUtf8(_pendingOutputFormat);
    record.imageData = _pendingImageBytes;

    std::string error;

    if (!_repository.InsertProcessedImage(record, error))
    {
        AfxMessageBox(L"DB 저장 실패: " + StringUtil::Utf8ToCString(error));
        SetStatusText(L"상태: 저장 실패");
        return;
    }

    ReloadProcessedImageList();
    SetStatusText(L"상태: DB 저장 완료");

    // 저장 완료 후 pending 상태 해제
    _hasPendingResult = false;
}

/*
    ------------------------------------------------------------------------
    OnBnClickedButtonDelete
    ------------------------------------------------------------------------
    [삭제] 버튼 클릭 시 동작

    흐름:
    1. 현재 선택된 리스트 항목 ID 확인
    2. DB에서 해당 행 삭제
    3. 리스트 새로고침
    4. 결과 이미지 영역 초기화
*/
void CMfcProgramDlg::OnBnClickedButtonDelete()
{
    int id = GetSelectedRecordId();

    if (id <= 0)
    {
        AfxMessageBox(L"삭제할 항목을 선택하세요.");
        return;
    }

    std::string error;

    if (!_repository.DeleteProcessedImage(id, error))
    {
        AfxMessageBox(L"삭제 실패: " + StringUtil::Utf8ToCString(error));
        SetStatusText(L"상태: 삭제 실패");
        return;
    }

    ReloadProcessedImageList();
    ClearResultImage();
    SetStatusText(L"상태: 삭제 완료");
}

/*
    ------------------------------------------------------------------------
    OnBnClickedButtonRefresh
    ------------------------------------------------------------------------
    [새로고침] 버튼 클릭 시 동작

    현재 DB 기준으로 리스트를 다시 읽어서 갱신
*/
void CMfcProgramDlg::OnBnClickedButtonRefresh()
{
    ReloadProcessedImageList();
    SetStatusText(L"상태: 리스트 새로고침 완료");
}

/*
    ------------------------------------------------------------------------
    OnLvnItemchangedListResults
    ------------------------------------------------------------------------
    리스트에서 항목 선택이 바뀌었을 때 호출

    흐름:
    1. 선택된 항목의 ID 읽기
    2. DB에서 단일 레코드 상세 조회
    3. BLOB 이미지 바이트를 TEMP 파일로 복원
    4. Picture Control에 표시
    5. 제목 / URL 입력칸도 같이 채워줌

    참고:
    - 지금은 operation_summary를 다시 체크박스 상태로 복원하지는 않음
    - 필요하면 나중에 파싱해서 UI 복원 가능
*/
void CMfcProgramDlg::OnLvnItemchangedListResults(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    *pResult = 0;

    // 선택 상태로 바뀐 경우만 처리
    if ((pNMLV->uNewState & LVIS_SELECTED) == 0)
    {
        return;
    }

    int id = GetSelectedRecordId();
    if (id <= 0)
    {
        return;
    }

    ProcessedImageRecord record;
    std::string error;

    if (!_repository.GetProcessedImageById(id, record, error))
    {
        SetStatusText(L"상태: 상세 조회 실패");
        return;
    }

    // 기본 정보는 입력창에도 표시
    _editTitle.SetWindowTextW(StringUtil::Utf8ToCString(record.title));
    _editImageUrl.SetWindowTextW(StringUtil::Utf8ToCString(record.imageUrl));

    // BLOB 이미지 바이트를 임시 파일로 만든 뒤 화면 표시
    CString tempPath;
    if (TempImageUtil::SaveBytesToTempFile(
        record.imageData,
        StringUtil::Utf8ToCString(record.outputFormat),
        tempPath))
    {
        ShowImageInControl(tempPath, _staticResultImage, _hResultBitmap);
        SetStatusText(L"상태: 저장된 결과 이미지 표시 완료");
    }
}

/*
    ------------------------------------------------------------------------
    OnPaint / OnQueryDragIcon
    ------------------------------------------------------------------------
    MFC 기본 아이콘 처리 코드
*/
void CMfcProgramDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);

        CRect rect;
        GetClientRect(&rect);

        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        dc.DrawIcon(x, y, _hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

HCURSOR CMfcProgramDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(_hIcon);
}
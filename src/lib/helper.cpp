#include "expander.h"
#include "lib/string.h"
#include "lib/terminal.h"

extern "C" {
#include "etc/md5.h"
}


/**
 * Gibt den letzten aufgetretenen Windows-Fehler des aktuellen Threads zur�ck. Wrapper f�r kernel32::GetLastError(),
 * da MQL eine Funktion desselben Namens definiert.
 *
 * @return int - Fehlercode
 */
int WINAPI GetLastWin32Error() {
   return(GetLastError());
   #pragma EXPANDER_EXPORT
}


/**
 * Ob die angegebene Timeframe-ID einen MetaTrader-Standard-Timeframe darstellt.
 *
 * @param  int timeframe - Timeframe-ID
 *
 * @return BOOL
 */
BOOL WINAPI IsStdTimeframe(int timeframe) {
   switch (timeframe) {
      case PERIOD_M1 : return(TRUE);
      case PERIOD_M5 : return(TRUE);
      case PERIOD_M15: return(TRUE);
      case PERIOD_M30: return(TRUE);
      case PERIOD_H1 : return(TRUE);
      case PERIOD_H4 : return(TRUE);
      case PERIOD_D1 : return(TRUE);
      case PERIOD_W1 : return(TRUE);
      case PERIOD_MN1: return(TRUE);
   }
   return(FALSE);
   #pragma EXPANDER_EXPORT
}


/**
 * Ob die angegebene Timeframe-ID einen benutzerdefinierten Timeframe darstellt.
 *
 * @param  int timeframe - Timeframe-ID
 *
 * @return BOOL
 */
BOOL WINAPI IsCustomTimeframe(int timeframe) {
   if (timeframe <= 0)
      return(FALSE);
   return(!IsStdTimeframe(timeframe));
   #pragma EXPANDER_EXPORT
}


/**
 * Gibt die ID des Userinterface-Threads zur�ck.
 *
 * @return DWORD - Thread-ID (nicht das Thread-Handle) oder 0, falls ein Fehler auftrat
 */
DWORD WINAPI GetUIThreadId() {
   static DWORD uiThreadId;

   if (!uiThreadId) {
      HWND hWnd = GetTerminalMainWindow();
      if (hWnd)
         uiThreadId = GetWindowThreadProcessId(hWnd, NULL);
   }
   return(uiThreadId);
   #pragma EXPANDER_EXPORT
}


/**
 * Whether the specified thread is the application's UI thread.
 *
 * @param  DWORD threadId [optional] - thread id; if this value is NULL the current thread is used
 *                                     (default: the current thread)
 * @return BOOL
 */
BOOL WINAPI IsUIThread(DWORD threadId/*= NULL*/) {
   if (!threadId)
      threadId = GetCurrentThreadId();
   return(threadId == GetUIThreadId());
   #pragma EXPANDER_EXPORT
}


/**
 * Wrapper f�r die Win32-API-Funktion GetProp(). Gibt den Wert einer Window-Property zur�ck.
 *
 * @param  HWND  hWnd   - Fensterhandle
 * @param  char* lpName - Property-Name
 *
 * @return HANDLE - Property-Value
 */
HANDLE WINAPI GetWindowProperty(HWND hWnd, const char* lpName) {
   return(GetProp(hWnd, lpName));
   #pragma EXPANDER_EXPORT
}


/**
 * Wrapper f�r die Win32-API-Funktion RemoveProp(). Gibt den Wert einer Window-Property zur�ck und l�scht sie.
 *
 * @param  HWND  hWnd   - Fensterhandle
 * @param  char* lpName - Property-Name
 *
 * @return HANDLE - Property-Value
 */
HANDLE WINAPI RemoveWindowProperty(HWND hWnd, const char* lpName) {
   return(RemoveProp(hWnd, lpName));
   #pragma EXPANDER_EXPORT
}


/**
 * Wrapper f�r die Win32-API-Funktion SetProp(). Setzt eine Window-Property.
 *
 * @param  HWND   hWnd   - Fensterhandle
 * @param  char*  lpName - Property-Name
 * @param  HANDLE value  - Property-Value
 *
 * @return BOOL - Erfolgsstatus
 */
BOOL WINAPI SetWindowProperty(HWND hWnd, const char* lpName, HANDLE value) {
   return(SetProp(hWnd, lpName, value));
   #pragma EXPANDER_EXPORT
}


/**
 * Shifted die Werte eines IndicatorBuffers um eine Anzahl von Bars nach hinten. Die �ltesten Werte verfallen.
 *
 * @param  double buffer[]   - MQL-Double-Array (IndicatorBuffer)
 * @param  int    bufferSize - Gr��e des Arrays
 * @param  int    bars       - Anzahl der zu shiftenden Bars
 * @param  double emptyValue - Initialisierungswert f�r freiwerdende Bufferelemente
 *
 * @return BOOL - Erfolgsstatus
 */
BOOL WINAPI ShiftIndicatorBuffer(double buffer[], int bufferSize, int bars, double emptyValue) {
   if (buffer && (uint)buffer < MIN_VALID_POINTER) return(error(ERR_INVALID_PARAMETER, "invalid parameter buffer: 0x%p (not a valid pointer)", buffer));
   if (bufferSize < 0)                             return(error(ERR_INVALID_PARAMETER, "invalid parameter bufferSize: %d", bufferSize));
   if (bars < 0)                                   return(error(ERR_INVALID_PARAMETER, "invalid parameter bars: %d", bars));
   if (!bufferSize || !bars) return(TRUE);

   MoveMemory((void*)&buffer[0], &buffer[bars], (bufferSize-bars)*sizeof(buffer[0]));

   for (int i=bufferSize-bars; i < bufferSize; i++) {
      buffer[i] = emptyValue;
   }
   return(TRUE);
   #pragma EXPANDER_EXPORT
}


/**
 * Copy the symbol-timeframe description as in the title bar of a chart window to the specified buffer. If the buffer is too
 * small the string in the buffer is truncated. The string is always terminated with a NULL character.
 *
 * @param  char* symbol
 * @param  uint  timeframe
 * @param  char* buffer
 * @param  uint  bufferSize
 *
 * @return uint - Amount of copied characters not counting the terminating NULL character or {bufferSize} if the buffer is
 *                too small and the string in the buffer was truncated. NULL in case of errors.
 */
uint WINAPI GetChartDescription(const char* symbol, uint timeframe, char* buffer, uint bufferSize) {
   uint symbolLength = strlen(symbol);
   if (!symbolLength || symbolLength > MAX_SYMBOL_LENGTH) return(error(ERR_INVALID_PARAMETER, "invalid parameter symbol: %s", DoubleQuoteStr(symbol)));
   if (!buffer)                                           return(error(ERR_INVALID_PARAMETER, "invalid parameter buffer: %p", buffer));
   if ((int)bufferSize <= 0)                              return(error(ERR_INVALID_PARAMETER, "invalid parameter bufferSize: %d", bufferSize));

   char* sTimeframe = NULL;

   switch (timeframe) {
      case PERIOD_M1 : sTimeframe = "M1";      break;
      case PERIOD_M5 : sTimeframe = "M5";      break;
      case PERIOD_M15: sTimeframe = "M15";     break;
      case PERIOD_M30: sTimeframe = "M30";     break;
      case PERIOD_H1 : sTimeframe = "H1";      break;
      case PERIOD_H4 : sTimeframe = "H4";      break;
      case PERIOD_D1 : sTimeframe = "Daily";   break;
      case PERIOD_W1 : sTimeframe = "Weekly";  break;
      case PERIOD_MN1: sTimeframe = "Monthly"; break;
      default:
         return(error(ERR_INVALID_PARAMETER, "invalid parameter timeframe: %d", timeframe));
   }
   uint chars = _snprintf(buffer, bufferSize, "%s,%s", symbol, sTimeframe);

   if (chars < 0 || chars==bufferSize) {
      buffer[chars-1] = 0;
      return(bufferSize);
   }
   return(chars);
}


/**
 * Calculate the MD5 hash of the input.
 *
 * @param  void* input  - buffer with binary content
 * @param  uint  length - length of the content in bytes
 *
 * @return char* - MD5 hash or a NULL pointer in case of errors
 */
char* WINAPI MD5Hash(const void* input, uint length) {
   if ((uint)input < MIN_VALID_POINTER) return((char*)error(ERR_INVALID_PARAMETER, "invalid parameter input: 0x%p (not a valid pointer)", input));
   if (length < 1)                      return((char*)error(ERR_INVALID_PARAMETER, "invalid parameter length: %d", length));

   MD5_CTX context;
   MD5_INIT(&context);
   MD5_UPDATE(&context, input, length);
   uchar buffer[16];                                                 // on the stack
   MD5_FINAL((uchar*)&buffer, &context);                             // fill buffer with binary MD5 hash (16 bytes)

   std::stringstream ss;                                             // convert hash to hex string (32 chars)
   ss << std::hex;
   for (uint i=0; i < 16; i++) {
      ss << std::setw(2) << std::setfill('0') << (int)buffer[i];
   }

   return(strdup(ss.str().c_str()));                                 // TODO: add to GC (close memory leak)
   #pragma EXPANDER_EXPORT
}


/**
 * Calculate the MD5 hash of a C string (Ansi).
 *
 * @param  char* input - C input string
 *
 * @return char* - MD5 hash or a NULL pointer in case of errors
 */
char* WINAPI MD5HashA(const char* input) {
   if ((uint)input < MIN_VALID_POINTER) return((char*)error(ERR_INVALID_PARAMETER, "invalid parameter input: 0x%p (not a valid pointer)", input));

   return(MD5Hash(input, strlen(input)));
   #pragma EXPANDER_EXPORT
}


/**
 * Get a unique message id for the string "MetaTrader4_Internal_Message".
 *
 * @return uint - message id in the range from 0xC000 to 0xFFFF or 0 (zero) in case of errors
 */
uint WINAPI MT4InternalMsg() {
   static uint msgId;
   if (!msgId) {
      msgId = RegisterWindowMessageA("MetaTrader4_Internal_Message");
      if (!msgId) return(error(ERR_WIN32_ERROR+GetLastError(), "->RegisterWindowMessage()"));
   }
   return(msgId);
   #pragma EXPANDER_EXPORT
}


/**
 * Alias of MT4InternalMsg()
 *
 * Get a unique message id for the string "MetaTrader4_Internal_Message".
 *
 * @return uint - message id in the range from 0xC000 to 0xFFFF or 0 (zero) in case of errors
 */
uint WINAPI WM_MT4() {
   return(MT4InternalMsg());
   #pragma EXPANDER_EXPORT
}

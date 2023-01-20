#ifndef __SCREEN_H
#define __SCREEN_H

enum  TextDestination {
      UserText = 1,
      DataText,
      WarnText
    };

enum  StatusColumn {
      ProgramVer = 0,
      HostName,
      XferStatistics,
      OnlineTime,
      CurrentTime,
      LastColumn
    };

extern int  SCR_Init           (int first);
extern void SCR_Clear          (void);
extern void SCR_ClearEOL       (void);
extern void SCR_ClearBOL       (void);
extern void SCR_ClearBOD       (void);
extern void SCR_ClearEOD       (void);
extern void SCR_ScrollUp       (int, int, int, int);
extern void SCR_ScrollDown     (int, int, int, int);
extern void SCR_GotoRowCol     (int, int);
extern void SCR_SetScroll      (int, int);
extern void SCR_MoveLeft       (int);
extern void SCR_MoveRight      (int);
extern void SCR_MoveUp         (int);
extern void SCR_MoveDown       (int);
extern void SCR_MoveUpScroll   (void);
extern void SCR_MoveDownScroll (void);
extern int  SCR_GetRow         (void);
extern int  SCR_GetColumn      (void);
extern void SCR_SetColour      (int);
extern int  SCR_GetColour      (void);
extern void SCR_SetFore        (int);
extern int  SCR_GetFore        (void);
extern void SCR_SetBack        (int);
extern int  SCR_GetBack        (void);
extern void SCR_ColourDefault  (void);
extern void SCR_SetInverse     (void);

extern void SCR_PutChar        (unsigned char);
extern void SCR_PutString      (const char *fmt, ...);
extern void SCR_SetScrHeight   (int h);
extern void SCR_SetScrWidth    (int w);
extern int  SCR_GetScrHeight   (void);
extern int  SCR_GetScrWidth    (void);
extern void SCR_Wrap           (int o);
extern void SCR_ScrollColour   (int c);
extern void SCR_SetBold        (int o);
extern void SCR_SetBlink       (int o);
extern void SCR_InsertChar     (int n, int c);
extern void SCR_DeleteChar     (int n);
extern void SCR_InsertLine     (int w, int n);
extern void SCR_DeleteLine     (int w, int n);
extern void SCR_EraseChars     (int n);
extern void SCR_Fill           (int l, int t, int r, int b, int ch, int co);
extern void SCR_SetTabStop     (void);
extern void SCR_ClearTabStop   (void);
extern void SCR_ClearAllTabs   (void);
extern void SCR_PrintTab       (void);
extern void SCR_MapCharSet     (int);
extern void SCR_SetCharSet     (int, int);

extern void SCR_StatusLine (enum StatusColumn column, const char *fmt, ...);
extern void SCR_StatusFill (const char *fmt, ...);
extern void SetColour      (enum TextDestination dest);

#endif

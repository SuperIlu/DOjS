GLAPI BOOL GLWINAPI wglUseFontBitmapsA(HDC hdc, DWORD first,
                                       DWORD count, DWORD listBase)
{
   int i;
   GLuint font_list;
   DWORD size;
   GLYPHMETRICS gm;
   HANDLE hBits;
   LPSTR lpBits;
   MAT2 mat;

   if (first<0)
      return FALSE;
   if (count<0)
      return FALSE;
   if (listBase<0)
      return FALSE;

   font_list = listBase;

   mat.eM11 = FixedFromDouble(1);
   mat.eM12 = FixedFromDouble(0);
   mat.eM21 = FixedFromDouble(0);
   mat.eM22 = FixedFromDouble(-1);

   memset(&gm,0,sizeof(gm));

   for (i = 0; i < count; i++)
   {
       DWORD err;
       
      glNewList( font_list+i, GL_COMPILE );

      /* allocate space for the bitmap/outline */
      size = GetGlyphOutline(hdc, first + i, GGO_BITMAP, &gm, 0, NULL, &mat);
      if (size == GDI_ERROR)
      {
         err = GetLastError();
         return(FALSE);
      }

      hBits  = GlobalAlloc(GHND, size+1);
      lpBits = GlobalLock(hBits);

      err = 
      GetGlyphOutline(hdc,    /* handle to device context */
                      first + i,          /* character to query */
                      GGO_BITMAP,         /* format of data to return */
                      &gm,                /* pointer to structure for metrics*/
                      size,               /* size of buffer for data */
                      lpBits,             /* pointer to buffer for data */
                      &mat                /* pointer to transformation */
                                          /* matrix structure */
                  );

      if (err == GDI_ERROR)
      {
         err = GetLastError();

         GlobalUnlock(hBits);
         GlobalFree(hBits);

         return(FALSE);
      }

      glBitmap(gm.gmBlackBoxX,gm.gmBlackBoxY,
               gm.gmptGlyphOrigin.x,
               gm.gmptGlyphOrigin.y,
               gm.gmCellIncX,gm.gmCellIncY,
               (const GLubyte * )lpBits);

      GlobalUnlock(hBits);
      GlobalFree(hBits);

      glEndList( );
   }

   return TRUE;
}

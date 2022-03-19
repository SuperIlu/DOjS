#pragma once

typedef void *HTAP;

HTAP OpenTapInterface (const char *Name, const unsigned char MacAddr[6]);
bool ChangeMediaStatus (HTAP hTap, bool status);
void CloseTap (HTAP hTap);
int ReadPacket (HTAP hTap, unsigned char buffer[1500]);
bool SendPacket (HTAP hTap, unsigned char *buffer, int size);

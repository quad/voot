#ifndef __VCONSOLE_H__
#define __VCONSOLE_H__

void vc_puts(char *in_str);
int vc_printf(char *fmt, ...);
void vc_clear(int r, int g, int b);

#endif
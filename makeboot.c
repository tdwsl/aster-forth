#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
   char buf[256+1];
   FILE *in = fopen("aster.f","r");
   FILE *out = fopen("aster_boot.h","w");
   if (!in || !out) { puts("mislukt"); exit(1); }
   fprintf(out,"static char *aster_boot =\n");
   char ch;
   for (;;) {
      int i=0;
      for (;;) {
         ch=fgetc(in);
         if (ch == EOF || ch == '\n') break;
         if (ch=='"') { buf[i++] = '\\'; buf[i++]=ch; }
         else if (ch=='\\') { buf[i++] = '\\'; buf[i++]='\\'; }
         else buf[i++]=ch;
      }
      if (ch==EOF) break;
      buf[i]=0;
      if (buf[0]) fprintf(out,"\"%s\\n\"\n",buf);
   }
   fputc(';',out);
   fclose(in); fclose(out);
   return 0;
}

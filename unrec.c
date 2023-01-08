/********** UnRec V0.1 *******************/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <io.h>

typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned long DWORD;


void read_header(FILE *fp);


WORD head[13];
DWORD start_adr;
DWORD fpos, fpos1;

void main (int argc, char *vect[])
{
  FILE *fp;
  FILE *sp;

  WORD crypt1, crypt2, anz_cry, header_size, i;

  char *filename;
  BYTE dec_inc, anz, low, test;
  WORD buffer[0x20];
  BYTE buf[10];

  printf("\nUnRec V0.2 fÅr REC 0.04\n\n");
  if ((vect[1]==NULL) || (argc!=2))
    {
      printf("usage: UNREC <filename>\n\n");
      exit(1);
    }
  filename=vect[1];
  if ((fp = fopen(filename,"r+b")) == NULL) exit(0);

  read_header(fp);
  fpos = start_adr+6+0x2b8;                         /* pos auf 0x0F8A */
  fseek(fp, fpos, SEEK_SET);
  fread(buffer, sizeof buffer[0], 3, fp);
  crypt1 = buffer[0];
  dec_inc = (BYTE) (buffer[2]>>8);
  if ((sp = fopen("out.exe","wb")) == NULL) exit(0);
  anz_cry = 0x1c5;
  fpos -= 0x10;                                     /* pos auf 0x0F7A */
  fpos -= 0x1CB;                                    /* pos auf 0x0DAF */
  fseek(fp, fpos, SEEK_SET);

  for(i=0;i<anz_cry;i++)                /* Decrypt-Progr.-teil decrypten */
    {
     fread(buffer, sizeof buffer[0], 1, fp);
     buffer[0] = buffer[0] ^ crypt1;
     fseek(fp, fpos, SEEK_SET);
     fwrite(&buffer[0], sizeof buffer[0], 1, fp);
     (dec_inc==0x43) ? (crypt1++) : (crypt1--);
     fpos++;
     fseek(fp, fpos, SEEK_SET);
    }

  fpos = start_adr+6+0x1c7;                         /* pos auf 0x0E99 */
  fseek(fp, fpos, SEEK_SET);
  fread(buffer, sizeof buffer[0], 1, fp);
  crypt2 = buffer[0];

  fpos = start_adr+6+0x2ca-1;                       /* pos auf Anfang Header */
  fpos1 = 0x00;
  fseek(fp, fpos, SEEK_SET);           /* Header vom Ende an Anfang kopieren */
  do
   {
    if( (anz=fread(buffer, sizeof buffer[0], 0x10, fp))!=0x10)
      {
       fseek(fp, fpos1, SEEK_SET);
       fwrite(buffer, sizeof buffer[0], anz, fp);
       anz = 0x00;
      }
    else
      {
       fseek(fp, fpos1, SEEK_SET);
       fwrite(buffer, sizeof buffer[0], 0x10, fp);
       fpos1 += 0x20;
       fpos += 0x20;
      }
    fseek(fp, fpos, SEEK_SET);
   }while (anz != 0x00);
				  /* genaue Filelaenge+1 in start_adr */
  fpos=0;
  fseek(fp, fpos, SEEK_SET);
  fread(buffer, sizeof buffer[0], 5, fp);
  start_adr  = (DWORD)(buffer[2]-1) * 512;         /* Headergroesse in Byte */
  start_adr += (DWORD)buffer[1];

  fpos=0;
  fseek(fp, fpos, SEEK_SET);
  fseek(sp, fpos, SEEK_SET);
  do                                 /* <file> in out.exe kopieren */
   {
    fread(buffer, sizeof buffer[0], 1, fp);
    fwrite(buffer, sizeof buffer[0], 1, sp);
    fpos+=2;
    if((fpos+1) >= start_adr)
      {
       fseek(sp, fpos, SEEK_SET);
       fseek(fp, fpos, SEEK_SET);
       fread(buf, sizeof buf[0], 1, fp);
       fwrite(buf, sizeof buf[0], 1, sp);
       fpos+=2;
      }
    fseek(fp, fpos, SEEK_SET);
    fseek(sp, fpos, SEEK_SET);
   }while (fpos < start_adr);

  fpos = 8;                         /* pos auf Headergroesse */
  fseek(fp, fpos, SEEK_SET);
  fread(buffer, sizeof buffer[0], 1, fp);
  header_size = buffer[0] * 0x10;
  fpos = (DWORD)header_size;
  fseek(fp, fpos, SEEK_SET);

  do                                /* Programmteil entschluesseln */
   {
    fread(buffer, sizeof buffer[0], 1, fp);
    low  = (BYTE) (buffer[0] >>8);
    buffer[0] = (buffer[0] <<8) + low;
    buffer[0] = buffer[0] ^ crypt2;
    fseek(sp, fpos, SEEK_SET);
    fwrite(buffer, sizeof buffer[0], 1, sp);
    fpos+=2;
    if((fpos+1) >= start_adr)
      {
       fseek(sp, fpos, SEEK_SET);
       fseek(fp, fpos,SEEK_SET);
       fread(buf, sizeof buf[0], 1, fp);
       fwrite(buf, sizeof buf[0], 1, sp);
       fpos+=2;
      }
    fseek(fp, fpos, SEEK_SET);
   }while(fpos<start_adr);				       /* Neue EXE - Groesse */

  fclose (fp);
  fclose (sp);
}

void read_header(FILE *fp)
{
 fpos=0;
 fseek(fp, fpos, SEEK_SET);
 fread(head, sizeof head[0], 13, fp);
 start_adr  = (DWORD)head[4] * 0x10;         /* Headergroesse in Byte */
 start_adr += (DWORD)head[11] * 0x10;
 start_adr += head[10];

}

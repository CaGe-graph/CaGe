
//cc -O4 -o makelist -DMAINFUNCTION="int getyesno(int argc, char *argv[])" makelist.c vul_in.c


#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MINBOUNDARY 0
#define MAXBOUNDARY 35

#define BIT(i) ( 1ULL <<(i))
#define MASK64 (63ULL)
#define FIRSTBIT(boundary) (((boundary)[0])&1ULL)
#define SHIFT(boundary,lengte) \
{int _c, limit; limit=((lengte)-1)>>6; \
if (FIRSTBIT(boundary)) { \
  for (_c=0;_c<limit;_c++) {(boundary)[_c]=((boundary)[_c]>>1);\
                          if (FIRSTBIT((boundary)+_c+1)) S_SETBIT((boundary)+_c,63); }\
  (boundary)[limit]=(boundary)[limit]>>1; \
  S_SETBIT((boundary),lengte-1);}\
else { for (_c=0;_c<limit;_c++) {(boundary)[_c]=((boundary)[_c]>>1);\
                          if (FIRSTBIT((boundary)+_c+1)) S_SETBIT((boundary)+_c,63); }\
       (boundary)[limit]=(boundary)[limit]>>1; \
                         }}

#define S_SETBIT(boundary,position) ((boundary)[(position)>>6]|=BIT((position) & MASK64))




#define SKIP(length) ((1<<((length)-6))-2)    /* after how many bytes do the codes for length start? */
#define WHICHBYTE(number) ((int)(((unsigned long long int)(number))>>4))
/* in fact not "number" is analyzed, but "number" without the final 1 */
#define WHICHBIT(number) ((int)(((unsigned long long int)(number)>>1) & 7ULL))
//#define SETBIT(length,number) (binfield[SKIP(length)+WHICHBYTE(number)]|= (((unsigned char)1)<<WHICHBIT(number)))
#define ISPOSSIBLE(length,bitstring) \
(((length)>maxboundary) || ((length)<minboundary) ||\
(bbinfield[SKIP(length)+WHICHBYTE(*(bitstring))] & (((unsigned char)1)<<WHICHBIT(*(bitstring)))))
#define IMPOSSIBLE(length,bitstring) (!(ISPOSSIBLE(length,bitstring)))
/* these makros assume that the boundary fits into one variable of
   type ULL in cases where the existence is looked up. But of course it would NEVER be possible
   to come close to length 64 for storing information on which boundares exist...*/

#define SETBIT(length,number) (bbinfield[SKIP(length)+WHICHBYTE(*(number))]|= (((unsigned char)1)<<WHICHBIT(*(number))))
#define ISSET(length,number) (bbinfield[SKIP(length)+WHICHBYTE(*(number))] & (((unsigned char)1)<<WHICHBIT(*(number))))
#define NOTSET(length,number) (!( ISSET(length,(*(number)))))

#define IS_SET(boundary,position) ((((boundary)[(position)>>6] & BIT((position) & MASK64))==0ULL)?0:1)
#define IS_SET_ULL(boundary,position) ((boundary)[(position)>>6] & BIT((position) & MASK64))
#define NOT_SET(boundary,position) ((((boundary)[(position)>>6] & BIT((position) & MASK64))==0ULL)?1:0)
#define NOT_SET_ULL(boundary,position) (!((boundary)[(position)>>6] & BIT((position) & MASK64)))

unsigned char *bbinfield;
char testoption[5]="t";
int minboundary,maxboundary;

long long int pos=0LL, impos=0LL;

//declaration of main function
MAINFUNCTION;

void runtest(char string[], int position, int lengte, unsigned long long int bitstring)
{
  static char dummy[]="getyesno";
  //FILE *fil;
  int result;
  static long long int teller=0;
  char *argv[20];
 


  if (position==lengte-1)
    { teller++;
      argv[0]=dummy;
      argv[1]=string;
      argv[2]=testoption;
      string[position]='3';
      bitstring |= 1;
      string[lengte]=0;
      //sprintf(befehl,"getyesno %s",string);
      //if ((teller%10000000LL)==0) { fprintf(stderr,"befehl %lld: %s (length %d)\n",teller,befehl,lengte); }

      //if (strcmp("223223223223",string)!=0) return;

      result=getyesno(3,argv);

      if (result) result=1;

      //fprintf(stderr,"result=%d string=%s\n",result, string);

      if (result==1) { 
	//fprintf(stderr,"setting 1 for %s = %llu\n",string,bitstring);
SETBIT(lengte,&bitstring); pos++; /*fclose(fil);*/ return; }

      if (result==0) { /*fclose(fil);*/ impos++; return; }
      fprintf(stderr,"PROBLEM!\n"); exit(1); 
    }

  string[position]='2';
  runtest(string,position+1,lengte,bitstring);
  string[position]='3';
  bitstring |= 1<<(lengte-1-position);
  runtest(string,position+1,lengte,bitstring);

  return;

}

void write_boundary(unsigned long long int boundary[], int lengte)
{

  int i;
  //fprintf(stderr,"--------------------the--boundary----(length %d)----------------------------\n",lengte);
  for (i=lengte-1; i>=0; i--)
    {if ((i+1)%64==0) fprintf(stderr," "); 
    if (IS_SET(boundary,i)) fprintf(stderr,"3"); else  fprintf(stderr,"2");
    }
  fprintf(stderr,"\n");
  //fprintf(stderr,"----------------------------------------------------\n");

}


void checkdata(char string[], int position, int lengte, unsigned long long int bitstring)
{
  static char dummy[]="getyesno";
  //FILE *fil;
  int result, i;
  char *argv[20];
 


  if (position==lengte-1)
    { //teller++;
      //fprintf(stderr,"%d %d\n",position,lengte);
      argv[0]=dummy;
      argv[1]=string;
      argv[2]=testoption;
      string[position]='3';
      bitstring |= 1;
      string[lengte]=0;
      //sprintf(befehl,"getyesno %s",string);
      //if ((teller%10000000LL)==0) { fprintf(stderr,"befehl %lld: %s\n",teller,befehl); }

      result=getyesno(3,argv);

      if (result) { if (IMPOSSIBLE(lengte,&bitstring)) 
	{ fprintf(stderr,"ERROR!\n"); 
      fprintf(stderr,"%s : result %d IMPOSSIBLE %d\n",string,result,IMPOSSIBLE(lengte,&bitstring) ); 
	}
      }
      else
      { if (ISPOSSIBLE(lengte,&bitstring)) { fprintf(stderr,"ERROR 2!\n"); exit(0); } }

      /* now check whether all shifts give the same */

      //      fprintf(stderr,"testing boundary: (result %d)\n",result);
      //write_boundary(&bitstring,lengte);
      if (result)
	{ for (i=0; i<lengte; i++)
	  { SHIFT(&bitstring,lengte);

	  if (IS_SET(&bitstring,0) && NOT_SET(&bitstring,lengte-1) && NOT_SET(&bitstring,lengte-2))
	    { 
	      if(IMPOSSIBLE(lengte,&bitstring))
		{ fprintf(stderr,"contradictory data (1): \n"); write_boundary(&bitstring,lengte); exit(0); }
	    }
	  }
	}
	  else
	for (i=0; i<lengte; i++)
	  { SHIFT(&bitstring,lengte);
	  if (IS_SET(&bitstring,0) && NOT_SET(&bitstring,lengte-1) && NOT_SET(&bitstring,lengte-2))

	    { 
	      if (ISPOSSIBLE(lengte,&bitstring))
		{ fprintf(stderr,"contradictory data (2) \n"); write_boundary(&bitstring,lengte); exit(0); }
	    }
	  }
      return;
    }

  string[position]='2';
  checkdata(string,position+1,lengte,bitstring);
  string[position]='3';
  bitstring |= 1<<(lengte-1-position);
  checkdata(string,position+1,lengte,bitstring);

  return;

}

void checkdata2()
{
  unsigned char *bfield2=NULL;
  FILE *fil;
  int i;
  char filename[100];

  //comparing with existing data

  if (bfield2==NULL)
    {
      bfield2=malloc(SKIP(maxboundary+1)*sizeof(unsigned char));
      if (bfield2==NULL) { fprintf(stderr,"Can't allocate room for bfield2!\n"); exit(12); }
      
      //fprintf(stderr,"Allocated %d bytes for datafield.\n",SKIP(maxboundary+1));
      sprintf(filename,"bindata_boundaries_length_%d_%d.bz2",minboundary,maxboundary);
      fprintf(stderr,"Opening %s for comparison.\n",filename);
      //sprintf(filename,"bindata_boundaries_length_%d_%d",minboundary,maxboundary);
      fil=fopen(filename,"r");
      
      if (fil==NULL) 
	{ fprintf(stderr,"Can't open %s for reading -- won't test.\n",filename);
	return;
	}
      else
	{
	  fclose(fil);
	  sprintf(filename,"bunzip2 -c bindata_boundaries_length_%d_%d.bz2",minboundary,maxboundary);
	  fil=popen(filename,"r");
	  if (fil==NULL) 
	    { fprintf(stderr,"Can't open pipe %s for reading -- won't test.\n",filename);
	    return; 
	    }
	  else 
	    {
	      if(fread(bfield2,sizeof(unsigned char),SKIP(maxboundary+1),fil)!=SKIP(maxboundary+1))
		{ fprintf(stderr,"Pipe %s can be opened, but can't read enough items!\n",filename); exit(33); }
	      fprintf(stderr,"Using precomputed existence data up to boundary length %d\n",maxboundary);
	      fclose(fil);
	    }
	}
    }

  for (i=0;i<SKIP(maxboundary+1)*sizeof(unsigned char); i++)
    if (bbinfield[i]!=bfield2[i]) {fprintf(stderr,"problem with byte %d!\n",i); exit(0); }
  //else fprintf(stderr,"OK\n");
}

void uusage(char name[])

{ fprintf(stderr,"usage: %s maxboundary [t], e.g. %s 16\n",name,name);
 fprintf(stderr,"[t] makes the program perform some test\n"); 
 fprintf(stderr,"maxboundary should be at most 35 -- reasonable is at most 30\n");
 exit(0);
}

int main(int argc, char *argv[])
{

  int lengte, test=0;
  char string[MAXBOUNDARY+2];
  unsigned long long int bitstring=0ULL;
  char filename[100];
  FILE *fil;

  if (sizeof(unsigned long long int)!=8) 
{ fprintf(stderr,"sizeof(unsigned long long int) must be 8 not %lu\n",sizeof(unsigned long long int));
 exit(0); } 

  if (argc<2) uusage(argv[0]);

  //minboundary=atoi(argv[1]);
  minboundary=7;
  //maxboundary=atoi(argv[2]);
  maxboundary=atoi(argv[1]);

  if (argc==3)
    { if (strcmp(argv[2],"t")!=0) uusage(argv[0]); else test=1; }

  if ((minboundary<MINBOUNDARY) || (maxboundary>MAXBOUNDARY)) 
    { fprintf(stderr,"minboundary>=%d and maxboundary<=%d necessary\n",MINBOUNDARY,MAXBOUNDARY);
    exit(0); }

  bbinfield=calloc(SKIP(maxboundary+1),sizeof(unsigned char));

  if (bbinfield==NULL) { fprintf(stderr,"Can't allocate room for bbinfield!\n"); exit(12); }

  //fprintf(stderr,"Allocated %d bytes for bbinfield.\n",SKIP(maxboundary+1));

  string[0]=string[1]='2';

  for (lengte=minboundary, bitstring =0U;lengte<=maxboundary;lengte++)
    runtest(string,2,lengte,bitstring);

  //SETBIT(11,57);
  //for (i=0;i<SKIP(maxboundary+1);i++) for (j=0;j<8;j++) 
  //  fprintf(stderr,"%d : %d\n",8*i+j,bbinfield[i]&(((unsigned char)1)<<j));

if (test)
  {fprintf(stderr,"testing data\n");
   for (lengte=minboundary;lengte<=maxboundary;lengte++)
    checkdata(string,2,lengte,bitstring);
   checkdata2();
  }
  sprintf(filename,"bindata_boundaries_length_%d_%d",minboundary,maxboundary);

  fil=fopen(filename,"w");
  if (fil==NULL) { fprintf(stderr,"Can't open file %s for writing\n",filename); exit(0); }



  if (fwrite(bbinfield,sizeof(unsigned char),SKIP(maxboundary+1),fil)!=SKIP(maxboundary+1))
    { fprintf(stderr,"Can't write data to file\n"); exit(0); }
  fclose(fil);



  fprintf(stderr,"possible boundaries %lld impossible %lld\n",pos,impos);

  return 0;



}

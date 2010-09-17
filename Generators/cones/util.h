/*
 *  util.h
 *  
 *
 *  Created by Nico Van Cleemput on 25/05/09.
 *
 */

#ifndef _UTIL_H //if not defined
#define _UTIL_H

#define HALFFLOOR(n) (n%2==0 ? n/2 : (n-1)/2)

//define boolean
typedef int boolean;

#define TRUE 1
#define FALSE 0

//#define _DEBUG

#ifdef _DEBUG

#define DEBUGMSG(msg) fprintf(stderr, "%s:%u %s\n", __FILE__, __LINE__, msg);

#define DEBUGCONDITIONALMSG(condition, msg) if(condition) fprintf(stderr, "%s:%u %s\n", __FILE__, __LINE__, msg);

#define DEBUGDUMP(var, format) fprintf(stderr, "%s:%u %s=" format "\n", __FILE__, __LINE__, #var, var);

#define DEBUGARRAYDUMP(var, size, format) { \
                                            fprintf(stderr, "%s:%u %s= [" format, __FILE__, __LINE__, #var, var[0]);\
                                            int debugarraydumpcounter;\
                                            for(debugarraydumpcounter=1; debugarraydumpcounter<size-1; debugarraydumpcounter++){ \
                                                fprintf(stderr, ", " format, var[debugarraydumpcounter]);\
                                            }\
                                            fprintf(stderr, ", " format "]\n", var[size-1]);\
                                          }

#define DEBUGASSERT(assertion) if(!(assertion)) fprintf(stderr, "%s:%u Assertion failed: %s\n", __FILE__, __LINE__, #assertion);

#else

#define DEBUGMSG(msg)

#define DEBUGCONDITIONALMSG(condition, msg)

#define DEBUGDUMP(var, format)

#define DEBUGARRAYDUMP(var, size, format)

#define DEBUGASSERT(assertion)

#endif

#endif // end if not defined, and end the header file

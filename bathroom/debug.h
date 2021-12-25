#ifndef _DEBUG_H_
#define _DEBUG_H_

#define _DEBUG_
//#define _TEST_

#ifdef _DEBUG_
#define _PP(a) Serial.print(a);
#define _PL(a) Serial.println(a);
#define _PM(a)              \
    Serial.print(millis()); \
    Serial.print(": ");     \
    Serial.println(a);
#else
#define _PP(a)
#define _PL(a)
#define _PM(a)
#endif

#endif
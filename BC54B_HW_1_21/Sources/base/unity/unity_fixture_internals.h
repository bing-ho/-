//- Copyright (c) 2010 James Grenning and Contributed to Unity Project
/* ==========================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
========================================== */

#ifndef UNITY_FIXTURE_INTERNALS_H_
#define UNITY_FIXTURE_INTERNALS_H_

#ifndef __FAR
#define __FAR
#endif

typedef struct _UNITY_FIXTURE_T {
    int Verbose;
    unsigned int RepeatCount;
    const char *__FAR NameFilter;
    const char *__FAR GroupFilter;
} UNITY_FIXTURE_T;

typedef void unityfunction(void);
void UnityTestRunner(unityfunction *__FAR setup,
                     unityfunction *__FAR body,
                     unityfunction *__FAR teardown,
                     const char *__FAR printableName,
                     const char *__FAR group,
                     const char *__FAR name,
                     const char *__FAR file, int line);

void UnityIgnoreTest(const char *__FAR printableName,
                     const char *__FAR group,
                     const char *__FAR name);
void UnityMalloc_StartTest(void);
void UnityMalloc_EndTest(void);
UNITY_COUNTER_TYPE UnityFailureCount(void);
UNITY_COUNTER_TYPE UnityIgnoreCount(void);
UNITY_COUNTER_TYPE UnityTestsCount(void);
int UnityGetCommandLineOptions(int argc, const char **argv);
void UnityConcludeFixtureTest(void);

void UnityPointer_Set(void *__FAR *__FAR ptr, void *__FAR newValue);
void UnityPointer_UndoAllSets(void);
void UnityPointer_Init(void);

void UnityAssertEqualPointer(const void *__FAR expected,
                             const void *__FAR actual,
                             const char *__FAR msg,
                             const UNITY_LINE_TYPE lineNumber);

#endif /* UNITY_FIXTURE_INTERNALS_H_ */

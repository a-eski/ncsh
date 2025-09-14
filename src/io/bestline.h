#pragma once
#ifdef __cplusplus
extern "C" {
#endif

enum bestlinePromptDirType {
    DT_NORMAL,
    DT_SHORT,
    DT_NONE
};
void bestlinePromptInit(bool showUser, enum bestlinePromptDirType dirType);

typedef struct bestlineCompletions {
    unsigned long len;
    char **cvec;
} bestlineCompletions;

typedef void(bestlineCompletionCallback)(const char *, int,
                                         bestlineCompletions *);
typedef char *(bestlineHintsCallback)(const char *, const char **, const char **);
typedef void(bestlineFreeHintsCallback)(void *);
typedef unsigned(bestlineXlatCallback)(unsigned);
typedef void(bestlineOnHistoryLoadedCallback(const char *, int));
// typedef void(bestlineHistoryCleanCallback(const char *, int));
// typedef void(bestlineHistoryRemoveCallback(const char *, int));

void bestlineSetCompletionCallback(bestlineCompletionCallback *);
void bestlineSetHintsCallback(bestlineHintsCallback *);
void bestlineSetFreeHintsCallback(bestlineFreeHintsCallback *);
void bestlineAddCompletion(bestlineCompletions *, const char *);
void bestlineSetXlatCallback(bestlineXlatCallback *);
void bestlineSetOnHistoryLoadedCallback(bestlineOnHistoryLoadedCallback *);

char *bestline(const char *);
char *bestlineInit(const char *, const char *);
char *bestlineRaw(const char *, int, int);
char *bestlineRawInit(const char *, const char *, int, int);
char *bestlineWithHistory(const char *, const char *);
int bestlineHistoryAdd(const char *);
int bestlineHistoryLoad(const char *);
int bestlineHistorySave(const char *);
// int bestlineHistoryClean();
// int bestlineHistoryRemove(const char *, int);
void bestlineBalanceMode(char);
void bestlineEmacsMode(char);
void bestlineClearScreen(int);
void bestlineDisableRawMode(void);
void bestlineFree(void *);
void bestlineFreeCompletions(bestlineCompletions *);
void bestlineHistoryFree(void);
void bestlineLlamaMode(char);
void bestlineMaskModeDisable(void);
void bestlineMaskModeEnable(void);

void bestlineUserIO(int (*)(int, void *, int), int (*)(int, const void *, int),
                    int (*)(int, int, int));

int bestlineCharacterWidth(int);
char bestlineIsSeparator(unsigned);
char bestlineNotSeparator(unsigned);
char bestlineIsXeparator(unsigned);
unsigned bestlineUppercase(unsigned);
unsigned bestlineLowercase(unsigned);
long bestlineReadCharacter(int, char *, unsigned long);

#ifdef __cplusplus
}
#endif

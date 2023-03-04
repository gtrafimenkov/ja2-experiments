#ifndef _IMP_PERSOANLITY_QUIZ_H
#define _IMP_PERSOANLITY_QUIZ_H

#include "MouseInput.h"
#include "SGP/Types.h"

void EnterIMPPersonalityQuiz(void);
void RenderIMPPersonalityQuiz(void);
void ExitIMPPersonalityQuiz(void);
void HandleIMPPersonalityQuiz(const struct MouseInput mouse);

void BltAnswerIndents(INT32 iNumberOfIndents);

extern INT32 giCurrentPersonalityQuizQuestion;
extern INT32 iCurrentAnswer;

#endif

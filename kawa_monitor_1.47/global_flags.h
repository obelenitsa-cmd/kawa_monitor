#ifndef GLOBAL_FLAGS_H
#define GLOBAL_FLAGS_H

#include <atomic>

// Объявляем переменную для ВСЕХ файлов проекта.
// Благодаря std::atomic значение 0 и 1 будет мгновенно передаваться между 2 ядрами.
extern std::atomic<bool> isOtaStarted;

#endif

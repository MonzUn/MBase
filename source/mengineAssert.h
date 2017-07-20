#pragma once
#include <cassert>

#define ASSERT_MEngine_INITIALIZED {\
	const char* msgStart = "Calling MEngine::";\
	const char* msgEnd	= " before the engine has been initialized is not allowed!";\
	constexpr int msgSize = sizeof(*msgStart) + sizeof(__func__) + sizeof(*msgEnd); \
	char msg[msgSize];\
	strcat(msg, msgStart);\
	strcat(msg, __func__);\
	strcat(msg, msgEnd);\
	assert(IsInitialized() && msg);}
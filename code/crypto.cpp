#include <stdio.h>

#include <cstdint>
#include <stdio.h>
#include <string.h> // NOTE(hugo) : for memset

#ifdef __unix__
#include <sys/types.h>
#endif
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef bool b32;
typedef float r32;
typedef double r64;

typedef size_t memory_index;

#define ArrayCount(x) (sizeof((x))/(sizeof((x)[0])))
#define Assert(x) do{if(!(x)){ printf("******\nASSERT FAIL:: FILE: %s, LINE: %i, FUNCTION: %s.\n******\n", __FILE__, __LINE__, __FUNCTION__); *(int*)0=0;}}while(0)

#define global_variable static
#define internal static
#define local_persist static

#define InvalidCodePath Assert(!"InvalidCodePath");
#define InvalidDefaultCase default: {InvalidCodePath;} break

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#include <stdlib.h>
#define ReAllocateArray(Buffer, type, Size) (type *)ReAllocate_(Buffer, (Size) * sizeof(type))
#define AllocateArray(type, Size) (type *)Allocate_((Size) * sizeof(type))
#define AllocateStruct(type) AllocateArray(type, 1)
#define Free(Buffer) free(Buffer)
#define CopyArray(Dest, Source, type, Size) memcpy(Dest, Source, (Size) * sizeof(type))

void* Allocate_(size_t Size)
{
	void* Result = malloc(Size);
	Assert(Result);
	memset(Result, 0, Size);

	return(Result);
}

void* ReAllocate_(void* Buffer, size_t Size)
{
	void* Result = realloc(Buffer, Size);
	Assert(Result);

	return(Result);
}
// }

inline u8
HexCharToByte(char C)
{
	if(C >= '0' && C <= '9')
	{
		return(C - '0');
	}
	else if(C >= 'a' && C <= 'f')
	{
		return(C - 'a' + 10);
	}
	else if(C >= 'A' && C <= 'F')
	{
		return(C - 'A' + 10);
	}
	InvalidCodePath;
	return(0);
}

inline char
ByteToBase64Char(u8 Byte)
{
	Assert(Byte < 64);
	if(Byte < 26)
	{
		return(Byte + 'A');
	}
	else if(Byte < 52)
	{
		return(Byte - 26 + 'a');
	}
	else if(Byte < 62)
	{
		return(Byte - 52 + '0');
	}
	else if(Byte == 62)
	{
		return('+');
	}
	else if(Byte == 63)
	{
		return('/');
	}
	InvalidCodePath;
	return(0);
}

internal char*
HexToBase64(char* HexString, memory_index Length)
{
	// NOTE(hugo): Since 3 Hex outputs 2 Base64, we need
	// to have a proper relation between the two.
	Assert(Length % 3 == 0);
	memory_index Base64StringLength = 2 * Length / 3;
	char* Result = AllocateArray(char, Base64StringLength + 1);
	char* C = Result;
	for(memory_index HexIndex = 0; HexIndex < Length - 2; HexIndex += 3)
	{
		u8 HexByte0 = HexCharToByte(HexString[HexIndex]);
		u8 HexByte1 = HexCharToByte(HexString[HexIndex + 1]);
		u8 HexByte2 = HexCharToByte(HexString[HexIndex + 2]);
		u8 Base64Byte0 = (HexByte0 << 2 | HexByte1 >> 2) & 0x3F;
		u8 Base64Byte1 = (HexByte1 << 4 | HexByte2) & 0x3F;
		*C = ByteToBase64Char(Base64Byte0);
		++C;
		*C = ByteToBase64Char(Base64Byte1);
		++C;
	}

	Result[Base64StringLength] = 0;
	return(Result);
}

inline u8
ByteFromTwoHex(char A, char B)
{
	u8 Top = HexCharToByte(A) << 4;
	u8 Bottom = HexCharToByte(B) & 0x0F;
	return(Top | Bottom);
}

// NOTE(hugo): This functions _always_
// output lowercase for a->f
inline char
HexCharFromByte(u8 Byte)
{
	Assert(Byte <= 0xF);
	if(Byte < 10)
	{
		return(Byte + '0');
	}
	else
	{
		return(Byte - 10 + 'a');
	}
}

inline void
CharHexFromByte(u8 Byte, char* A, char* B)
{
	u8 Top = Byte >> 4;
	u8 Bottom = Byte & 0x0F;
	*A = HexCharFromByte(Top);
	*B = HexCharFromByte(Bottom);
}

internal char*
FixedXOR(char* InputA, char* InputB, memory_index Length)
{
	char* Result = AllocateArray(char, Length + 1);
	char* CharResult = Result;

	for(memory_index CharIndex = 0; CharIndex < Length - 1; CharIndex += 2)
	{
		char CharA0 = InputA[CharIndex];
		char CharA1 = InputA[CharIndex + 1];

		char CharB0 = InputB[CharIndex];
		char CharB1 = InputB[CharIndex + 1];

		u8 ByteA = ByteFromTwoHex(CharA0, CharA1);
		u8 ByteB = ByteFromTwoHex(CharB0, CharB1);
		u8 ByteResult = ByteA ^ ByteB;
		CharHexFromByte(ByteResult, CharResult, CharResult + 1);
		CharResult += 2;
	}

	Result[Length] = 0;
	return(Result);
}

internal void
Test_Set1_Challenge1()
{
	char* Input = "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d";
	memory_index Length = strlen(Input);
	char* ExpectedResult = "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t";
	char* Result = HexToBase64(Input, Length);

	Assert(strlen(ExpectedResult) == strlen(Result));
	Assert(strcmp(Result, ExpectedResult) == 0);

	Free(Result);
}

internal void
Test_Set1_Challenge2()
{
	char* InputA = "1c0111001f010100061a024b53535009181c";
	memory_index Length = strlen(InputA);
	char* InputB = "686974207468652062756c6c277320657965";
	memory_index LengthB = strlen(InputB);
	Assert(LengthB == Length);
	char* Expected = "746865206b696420646f6e277420706c6179";
	char* Result = FixedXOR(InputA, InputB, Length);

	Assert(strlen(Result) == strlen(Expected));
	Assert(strcmp(Result, Expected) == 0);

	Free(Result);
}

int main(int ArgumentCount, char** Arguments)
{
	Test_Set1_Challenge1();
	Test_Set1_Challenge2();
	return(0);
}

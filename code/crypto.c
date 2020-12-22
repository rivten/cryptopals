#include <stdio.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h> // NOTE(hugo) : for memset
#include <stdbool.h>

#include <math.h>

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
#define Free(Buffer) free(Buffer);
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


#define MAX(a, b) ((a) > (b)) ? (a) : (b)

float Square(float V)
{
	return(V * V);
}

const char* ReadFileContent(const char* Filename)
{
#ifdef _WIN32
	FILE* File = 0;
	fopen_s(&File, Filename, "rb");
#else
	FILE* File = fopen(Filename, "rb");
#endif
	Assert(File);

	fseek(File, 0, SEEK_END);
	size_t FileSize = ftell(File);
	fseek(File, 0, SEEK_SET);
	char* Content = AllocateArray(char, FileSize + 1);
	const size_t ReadSize = fread(Content, 1, FileSize, File);
	Assert(ReadSize == FileSize);
	fclose(File);

	Content[FileSize] = 0;

	return(Content);
}

u8 HexCharToByte(char C)
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

char ByteToBase64Char(u8 Byte)
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

// NOTE(hugo): returned string to be freed by user
internal char* HexToBase64(const char* HexString, memory_index Length)
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

u8 ByteFromTwoHex(char A, char B)
{
	u8 Top = HexCharToByte(A) << 4;
	u8 Bottom = HexCharToByte(B) & 0x0F;
	return(Top | Bottom);
}

// NOTE(hugo): This functions _always_
// output lowercase for a->f
char HexCharFromByte(u8 Byte)
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

void CharHexFromByte(u8 Byte, char* A, char* B)
{
	u8 Top = Byte >> 4;
	u8 Bottom = Byte & 0x0F;
	*A = HexCharFromByte(Top);
	*B = HexCharFromByte(Bottom);
}

// NOTE(hugo): returned string to be freed by user
internal char* FixedXOR(const char* InputA, const char* InputB, memory_index Length)
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

bool IsAlphanumerical(char C)
{
	bool IsLetter = (C >= 'a' && C <= 'z') ||
		(C >= 'A' && C <= 'Z');
	bool IsNumber = (C >= '0' && C <= '9');
	return(IsLetter || IsNumber);
}

bool IsAlphabetical(char C)
{
	bool Result = (C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z');
	return(Result);
}

char ToLower(char C)
{
	Assert(IsAlphabetical(C));
	if(C >= 'A' && C <= 'Z')
	{
		C = C - 'A' + 'a';
	}

	return(C);
}

bool IsEnglishCharacter(char C)
{
	return(IsAlphanumerical(C) || C == ' ');
}

global_variable float EnglishLetterFrequency[26] = 
{
    0.08167, 0.01492, 0.02782, 0.04253, 0.12702, 0.02228, 0.02015,  // A-G
    0.06094, 0.06966, 0.00153, 0.00772, 0.04025, 0.02406, 0.06749,  // H-N
    0.07507, 0.01929, 0.00095, 0.05987, 0.06327, 0.09056, 0.02758,  // O-U
    0.00978, 0.02360, 0.00150, 0.01974, 0.00074                     // V-Z
};

internal float XORCypher_ComputeScore(const char* Str, memory_index Length, memory_index InputLength)
{
	float ChiSquare = 0.0f;

	u32 StrLetterSize = 0;
	u32 BadLetter = 0;
	for(u32 Index = 0; Index < Length; ++Index)
	{
		char C = Str[Index];
		if(IsAlphabetical(C))
		{
			++StrLetterSize;
		}
		else if(!IsEnglishCharacter(C))
		{
			++BadLetter;
		}
	}

	if(StrLetterSize == 0)
	{
		return(0.0f);
	}

	for(u32 LetterIndex = 0; LetterIndex < 26; ++LetterIndex)
	{
		u32 ObservationCount = 0;
		char CurrentChar = 'a' + LetterIndex;
		for(u32 Index = 0; Index < Length; ++Index)
		{
			char C = Str[Index];
			if(IsAlphabetical(C) &&
					ToLower(C) == CurrentChar)
			{
				++ObservationCount;
			}
		}

		float ExpectedCount = (float)(StrLetterSize) * EnglishLetterFrequency[LetterIndex];
		ChiSquare += Square((float)(ObservationCount) - ExpectedCount) / ExpectedCount;
	}

	float t = (1.0f + (float)(BadLetter)) *
		(1.0f + fabs((float)(InputLength - Length))) * ChiSquare;
	float FinalScore = 1.0f / (1.0f + t);

	return(FinalScore);
}

typedef struct xor_decipher_result
{
	float Score;
	char* DecodedStr; // NOTE(hugo): to be freed by the user
} xor_decipher_result;

internal xor_decipher_result SingleByteXORCypher(const char* Input, memory_index Length)
{
	Assert(Length % 2 == 0);
	xor_decipher_result Result = {};
	Result.DecodedStr = AllocateArray(char, Length / 2 + 1);

	char* CurrentString = AllocateArray(char, Length / 2 + 1);

	for(u32 CharIndex = 0; CharIndex <= 0xFF; ++CharIndex)
	{
		u8 CipherByte = CharIndex & 0xFF;

		char* C = CurrentString;
		for(memory_index Index = 0; Index < Length - 1; Index += 2)
		{
			u8 InputByte = ByteFromTwoHex(Input[Index], Input[Index + 1]);
			u8 ByteResult = InputByte ^ CipherByte;
			*C = ByteResult;
			++C;
		}
		float Score = XORCypher_ComputeScore(CurrentString, strlen(CurrentString),
				Length / 2);
		if(Score > Result.Score)
		{
			Result.Score = Score;
			memcpy(Result.DecodedStr, CurrentString, Length / 2 + 1);
		}
		memset(CurrentString, 0, Length / 2 + 1);
	}

	Result.DecodedStr[Length] = 0;
	Free(CurrentString);
	Assert(Result.Score != 0);
	return(Result);
}

bool IsEndLine(const char* C)
{
	return((C && *C == '\r' && (C + 1) && C[1] == '\n') || (C && *C == '\n'));
}

memory_index GetLineLength(const char* Str)
{
	memory_index Result = 0;
	const char* C = Str;
	while(!IsEndLine(C))
	{
		++Result;
		++C;
	}
	return(Result);
}

// NOTE(hugo): returned string to be deleted by user
internal const char* DecryptFile(const char* FileContent)
{
	const char* CurrentFilePos = FileContent;
	float BestScore = 0;
	const char* BestScoreResult = 0;
	while(*CurrentFilePos != 0)
	{
		const memory_index LineLength = GetLineLength(CurrentFilePos);
		const xor_decipher_result DecipherResult = SingleByteXORCypher(CurrentFilePos, LineLength);
		if(BestScore < DecipherResult.Score)
		{
			BestScore = DecipherResult.Score;
			BestScoreResult = DecipherResult.DecodedStr;
		}
        else
        {
            Free(DecipherResult.DecodedStr);
        }

		CurrentFilePos += (LineLength + 1);
		if(*CurrentFilePos == '\n')
		{
			++CurrentFilePos;
		}
	}
	return(BestScoreResult);
}

// NOTE(hugo): returned string to be freed by user
internal char* RepeatingKeyXOR(char* Input, memory_index Length, char* Key, memory_index KeyLength)
{
	char* Result = AllocateArray(char, 2 * Length + 1);
	char* ResultC = Result;

	for(u32 CharIndex = 0; CharIndex < Length; ++CharIndex)
	{
		char C = Input[CharIndex];
		char KeyChar = Key[CharIndex % KeyLength];
		u8 Byte = C ^ KeyChar;
		CharHexFromByte(Byte, ResultC, ResultC + 1);
		ResultC += 2;
	}

	Result[2 * Length] = 0;
	return(Result);
}

#define ASSERT_STR_IDENTICAL(A, B)\
	Assert(A); Assert(B);\
	Assert(strlen(A) == strlen(B));\
	Assert(strcmp((A), (B)) == 0);

internal void Test_Set1_Challenge1()
{
	const char* Input = "49276d206b696c6c696e6720796f757220627261696e206c696b65206120706f69736f6e6f7573206d757368726f6f6d";
	memory_index Length = strlen(Input);
	const char* ExpectedResult = "SSdtIGtpbGxpbmcgeW91ciBicmFpbiBsaWtlIGEgcG9pc29ub3VzIG11c2hyb29t";
	const char* Result = HexToBase64(Input, Length);

	ASSERT_STR_IDENTICAL(Result, ExpectedResult);

	Free(Result);
}

internal void Test_Set1_Challenge2()
{
	const char* InputA = "1c0111001f010100061a024b53535009181c";
	memory_index Length = strlen(InputA);
	const char* InputB = "686974207468652062756c6c277320657965";
	memory_index LengthB = strlen(InputB);
	Assert(LengthB == Length);
	const char* Expected = "746865206b696420646f6e277420706c6179";
	const char* Result = FixedXOR(InputA, InputB, Length);

	ASSERT_STR_IDENTICAL(Result, Expected);

	Free(Result);
}

internal void Test_Set1_Challenge3()
{
	const char* Input = "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736";
	memory_index Length = strlen(Input);
	xor_decipher_result Result = SingleByteXORCypher(Input, Length);
	printf("Result of Set1/Challenge3: %s\n", Result.DecodedStr);

	Free(Result.DecodedStr);
}

internal void Test_Set1_Challenge4()
{
	const char* InputFilename = "../data/4.txt";
	const char* FileContent = ReadFileContent(InputFilename);
	const char* Result = DecryptFile(FileContent);
	Free(FileContent);
	printf("Result of Set1/Challenge4: %s\n", Result);
    Free(Result);

}

internal void Test_Set1_Challenge5()
{
	const char* Input = "Burning 'em, if you ain't quick and nimble\nI go crazy when I hear a cymbal";
	memory_index Length = strlen(Input);

	const char* Key = "ICE";
	memory_index KeyLength = strlen(Key);

	const char* Result = RepeatingKeyXOR(Input, Length, Key, KeyLength);

	const char* Expected = "0b3637272a2b2e63622c2e69692a23693a2a3c6324202d623d63343c2a26226324272765272a282b2f20430a652e2c652a3124333a653e2b2027630c692b20283165286326302e27282f";

	ASSERT_STR_IDENTICAL(Result, Expected);

	Free(Result);
}

int main(int ArgumentCount, char** Arguments)
{
	Test_Set1_Challenge1();
	Test_Set1_Challenge2();
	Test_Set1_Challenge3();
	Test_Set1_Challenge4();
	Test_Set1_Challenge5();
	return(0);
}

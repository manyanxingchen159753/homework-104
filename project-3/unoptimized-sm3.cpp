#include <string.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <memory>
#include <stdint.h>
#include <ctime>
#include <ratio>
#include <chrono>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <iomanip>


using namespace std;
unsigned int hash_all = 0; //�ܵ���Ϣ��
unsigned int t[64];//��ǰ���������t��ֵ���д洢

#define MAXSIZE 1024 * 512 //������Ϣ��󳤶�

//�ж����л����Ƿ�ΪС��
static const int endianTest = 1;
#define IsLittleEndian() (*(char *)&endianTest == 1)
//����ѭ����λ
#define LeftShift(word, bits) ( (word) << (bits) | (word) >> (32 - (bits)) )
#pragma once

#define HASH_SIZE 32 //��ϣ�����Ĵ�С

namespace SM3 {

	typedef struct sm3_context_s {

		unsigned char MessageBlock[64]; //512λ�����ݿ�

		unsigned int iv[HASH_SIZE / 4];
	}sm3_context_t;

	unsigned char* Calculate(const unsigned char* message, unsigned int messageLen, unsigned char digest[HASH_SIZE]); //���㺯��
	std::vector<uint32_t> Implement_SM3(char* filepath); //ִ��sm3

}

//����Tj�����ĺ���ʵ��
unsigned int T(int i)
{
	if (i >= 0 && i <= 15)
		return 0x79cc4519;
	else if (i >= 16 && i <= 63)
		return 0x7a879d8a;
	else
		return 0;
}

//ʵ�ֲ�������FF����
unsigned int FF(unsigned int a, unsigned int b, unsigned int c, int i)
{
	if (i >= 0 && i <= 15)
		return a ^ b ^ c;
	else if (i >= 16 && i <= 63)
		return (a & b) | (a & c) | (b & c);
	else
		return 0;
}

//ʵ�ֲ�������GG����
unsigned int GG(unsigned int a, unsigned int b, unsigned int c, int i)
{
	if (i >= 0 && i <= 15)
		return a ^ b ^ c;
	else if (i >= 16 && i <= 63)
		return (a & b) | (~a & c);
	else
		return 0;
}

//P0
unsigned int P0(unsigned int a)
{
	return a ^ LeftShift(a, 9) ^ LeftShift(a, 17);
}

//��ת�ĸ��ֽڵ��ֽ���
unsigned int* ReverseWord(unsigned int* sequence)
{
	unsigned char* byte, temp;
	byte = (unsigned char*)sequence;

	temp = byte[0];
	byte[0] = byte[3];
	byte[3] = temp;

	temp = byte[1];
	byte[1] = byte[2];
	byte[2] = temp;

	return sequence;
}

//��ʼ������
void SM3_INIT(SM3::sm3_context_s* context) {
	context->iv[0] = 0x7380166f;
	context->iv[1] = 0x4914b2b9;
	context->iv[2] = 0x172442d7;
	context->iv[3] = 0xda8a0600;
	context->iv[4] = 0xa96f30bc;
	context->iv[5] = 0x163138aa;
	context->iv[6] = 0xe38dee4d;
	context->iv[7] = 0xb0fb0e4e;
}

//������Ϣ��
void SM3_ProcessMessageBlock(SM3::sm3_context_s* context)
{
	int i;
	unsigned int W1[68];
	unsigned int W2[64];
	unsigned int A, B, C, D, E, F, G, H, SS1, SS2, TT1, TT2;

	//��Ϣ��չ
	for (i = 0; i < 16; i++)
	{
		W1[i] = *(unsigned int*)(context->MessageBlock + i * 4);
		if (IsLittleEndian())
			ReverseWord(W1 + i);
	}
	for (i = 16; i < 68; i++)
	{
		W1[i] = (W1[i - 16] ^ W1[i - 9] ^ LeftShift(W1[i - 3], 15)) ^ LeftShift((W1[i - 16] ^ W1[i - 9] ^ LeftShift(W1[i - 3], 15)), 15) ^ LeftShift((W1[i - 16] ^ W1[i - 9] ^ LeftShift(W1[i - 3], 15)), 23) ^ LeftShift(W1[i - 13], 7) ^ W1[i - 6];
	}
	for (i = 0; i < 64; i++)
	{
		W2[i] = W1[i] ^ W1[i + 4];
	}

	//��Ϣѹ��
	A = context->iv[0];
	B = context->iv[1];
	C = context->iv[2];
	D = context->iv[3];
	E = context->iv[4];
	F = context->iv[5];
	G = context->iv[6];
	H = context->iv[7];

	for (i = 0; i < 64; i++)
	{

		SS1 = LeftShift((LeftShift(A, 12) + E + LeftShift(T(i), i)), 7);
		SS2 = SS1 ^ LeftShift(A, 12);
		TT1 = FF(A, B, C, i) + D + SS2 + W2[i];
		TT2 = GG(E, F, G, i) + H + SS1 + W1[i];

		D = C;
		C = LeftShift(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = LeftShift(F, 19);
		F = E;
		E = P0(TT2);

	}
	context->iv[0] ^= A;
	context->iv[1] ^= B;
	context->iv[2] ^= C;
	context->iv[3] ^= D;
	context->iv[4] ^= E;
	context->iv[5] ^= F;
	context->iv[6] ^= G;
	context->iv[7] ^= H;
}

//SM3��Ϣ�������㺯��
unsigned char* SM3::Calculate(const unsigned char* message,
	unsigned int MessageLen, unsigned char digest[HASH_SIZE])
{
SM3:sm3_context_s context;
	unsigned int i, r, len;

	SM3_INIT(&context);
	hash_all = MessageLen / 64 + 1;
	r = MessageLen % 64;
	if (r > 55) {
		hash_all += 1;
	}

	for (i = 0; i < MessageLen / 64; i++)
	{
		//��ǰ�����Ϣ������д���
		memcpy(context.MessageBlock, message + i * 64, 64);
		SM3_ProcessMessageBlock(&context);
	}

	//�����Ϣ����
	len = MessageLen * 8;
	if (IsLittleEndian())
		ReverseWord(&len);
	memcpy(context.MessageBlock, message + i * 64, r);
	context.MessageBlock[r] = 0x80;//��ĩβ����0x80����0x10000000
	if (r <= 55)//���ʣ�µ�λ������440
	{
		memset(context.MessageBlock + r + 1, 0, 64 - r - 1 - 8 + 4);
		memcpy(context.MessageBlock + 64 - 4, &len, 4);
		SM3_ProcessMessageBlock(&context);
	}
	else
	{
		memset(context.MessageBlock + r + 1, 0, 64 - r - 1);
		SM3_ProcessMessageBlock(&context);
		memset(context.MessageBlock, 0, 64 - 4);
		memcpy(context.MessageBlock + 64 - 4, &len, 4);
		SM3_ProcessMessageBlock(&context);
	}

	if (IsLittleEndian())
		for (i = 0; i < 8; i++)
			ReverseWord(context.iv + i);
	memcpy(digest, context.iv, HASH_SIZE);

	return digest;
}

//ִ��SM3����test�е����ݲ���ü�����
std::vector<uint32_t> SM3::Implement_SM3(char* filepath)
{
	std::vector<uint32_t> hash_result(32, 0);
	std::ifstream fin;
	uint32_t filesize = 0;
	unsigned char* buffer = new unsigned char[MAXSIZE];
	unsigned char hash_output[32];
	//��ȡ�ļ��Ĵ�С
	struct _stat info;
	_stat(filepath, &info);
	filesize = info.st_size;

	fin.open(filepath, std::ifstream::binary);
	fin >> buffer;
	fin.close();
	std::cout << "���������ݣ�" << buffer << endl;
	auto start = std::chrono::high_resolution_clock::now();
	SM3::Calculate(buffer, filesize, hash_output);
	auto end = std::chrono::high_resolution_clock::now();
	// �Ժ���Ϊ��λ����������ʱ��
	std::chrono::duration<double, std::ratio<1, 1000>> diff = end - start;
	std::cout << "��ʱ: " << diff.count() << " ms\n";

	hash_result.assign(&hash_output[0], &hash_output[32]);

	delete[]buffer;
	return hash_result;
}

int main() {
	char filepath[] = "C:/Users/DELL/Desktop/ttt.txt";
	std::vector<uint32_t> hash_result;
	hash_result = SM3::Implement_SM3(filepath);
std:cout << "Hash-���: ";
	for (int i = 0; i < 32; i++) {
		std::cout << setiosflags(ios::uppercase) << std::hex << std::setw(2) << std::setfill('0') << hash_result[i];
		if (((i + 1) % 4) == 0) std::cout << " ";
	}
	std::cout << std::endl;
	return 0;
}
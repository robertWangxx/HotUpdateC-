/*
	���ԭ���˳��
	�ص���ԭ������ַ��Ŀ�ĺ�����ַ��ȡ�ķ���
	dumpbin�����ã������exe �õ�������ڵĵ�ַ(ʵ������ʱ��ַ��䵫��Ե�ַ����) ������ڵ�������(����ʱҲ���ֲ��� ���ڴ��������)��
	releaseģʽ��ĳЩ�����ᱻ�Ż��� ʹ��dumpbin������޷��õ��������  ���ú�������ʱ���޷��Ż���������ڵ�ַ ���øú�����ȡ��ַ
	x86��x64�µĺ�����ת���ָ��ʵ������
	ͨ��ɨ���������ȡ������ַ����x86 ɨ�跶Χ��С
	�����������������������@ILT�� ͨ���������ӱ�����ת
	�������ӹرգ��������ֱ���ڶ��崦
*/
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <windows.h>
#include "fun1.h"

//#pragma comment(linker, "/entry:testfun")

#define JMPCODE_LENGTH 5            //x86 ƽ̹�ڴ�ģʽ�£�������תָ���  
#define JMPCMD_LENGTH  1            //��е��0xe9����  
#define JMPCMD         0xe9         //��Ӧ����jmpָ�� 


//дjmp��x64�汾����
bool WriteJMP_x64(LPCVOID dwFrom, LPCVOID dwTo)
{
	if (dwFrom == dwTo)
		return false;
	if (!dwFrom || !dwTo)
		return false;

	DWORD_PTR dwAdr = (DWORD_PTR)dwFrom;
	DWORD_PTR dwAdrTo = (DWORD_PTR)dwTo;
	DWORD   ProtectVar;              // �������Ա���
	MEMORY_BASIC_INFORMATION MemInfo;    //�ڴ��ҳ������Ϣ

	// ȡ�ö�Ӧ�ڴ��ԭʼ����
	if (0 != VirtualQuery(dwFrom, &MemInfo, sizeof(MEMORY_BASIC_INFORMATION)))
	{
		if (VirtualProtect(MemInfo.BaseAddress, MemInfo.RegionSize, PAGE_EXECUTE_READWRITE, &MemInfo.Protect))
		{

			*(BYTE*)dwAdr = 0x68;
			dwAdr += 1;
			*(DWORD32*)dwAdr = DWORD32(dwAdrTo & 0xffffffff);
			dwAdr += 4;
			*(DWORD32*)dwAdr = DWORD32(0x042444c7);
			dwAdr += 4;
			*(DWORD32*)dwAdr = DWORD32(dwAdrTo >> 32);
			dwAdr += 4;
			*(BYTE*)dwAdr = 0xc3;

			// �Ļ�ԭ����
			VirtualProtect(MemInfo.BaseAddress, MemInfo.RegionSize, MemInfo.Protect, &ProtectVar);

			// �޸ĺ󣬻���Ҫˢ��cache
			//FlushInstructionCache(GetCurrentProcess(), dwFrom, JMPCODE_LENGTH);

			return true;
		}
	}

	return false;
	/*
	push ��ַ�ĵ�32λ
	mov dword ptr ss:[rsp+4],��ַ�ĸ�32λ
	ret
	*/

	//14 bytes
}

//jmp xxxx(��ָ��һ��ռ�� 5 bytes)  jmpָ��ռ 1 byte   ��Ե�ַռ 4 bytes  
//дjmp��x86�汾����  ��Ϊx86��(pNewFunc - pOrigFunc)��ֵ������4�ֽ�   x64��(pNewFunc - pOrigFunc)ֵ���ܳ���4�ֽڴ�С
bool WriteJMP_x86(LPCVOID pOrigFunc, LPCVOID pNewFunc)
{
	if (pOrigFunc == pNewFunc)
		return false;

	if (pOrigFunc && pNewFunc)
	{
		DWORD   ProtectVar;              // �������Ա���
		MEMORY_BASIC_INFORMATION MemInfo;    //�ڴ��ҳ������Ϣ

		// ȡ�ö�Ӧ�ڴ��ԭʼ����
		if (0 != VirtualQuery(pOrigFunc, &MemInfo, sizeof(MEMORY_BASIC_INFORMATION)))
		{
			if (VirtualProtect(MemInfo.BaseAddress, MemInfo.RegionSize, PAGE_EXECUTE_READWRITE, &MemInfo.Protect))
			{
				// ����ԭ���ݣ���ֹ������Ҫʹ��memcpy������ʹ�����ƽӿ�
				//__inner_memcpy((unsigned char*)str_instruct_back, (unsigned char*)pOrigFunc, JMPCODE_LENGTH);

				// �޸�Ŀ���ַָ��Ϊ jmp pNewFunc
				*(unsigned char*)pOrigFunc = JMPCMD;                                      //����API���ں��������ǰ��ע��jmp xxxx  
				*(DWORD*)((unsigned char*)pOrigFunc + JMPCMD_LENGTH) = (DWORD)pNewFunc - (DWORD)pOrigFunc - JMPCODE_LENGTH;

				// �Ļ�ԭ����
				VirtualProtect(MemInfo.BaseAddress, MemInfo.RegionSize, MemInfo.Protect, &ProtectVar);

				// �޸ĺ󣬻���Ҫˢ��cache
				FlushInstructionCache(GetCurrentProcess(), pOrigFunc, JMPCODE_LENGTH);

				return true;
			}
		}
	}
	return false;
}

/*extern "C" _declspec(dllexport)*/ void fun1()
{
	int n = 1 + 1;
	int m = n * 6;
	std::cout << "123" << std::endl;
}

extern "C" _declspec(dllexport) void fun_base()
{
	std::cout << "zzz" << std::endl;
}



/*extern "C" _declspec(dllexport)*/ void fun2()
{
	std::cout << "456" << std::endl;
	//fun1();		// ����fun2����fun1�� ѭ���ݹ�ķ�������
	fun_base();
}

typedef void(*pFun)();

class C
{
public:
	void f() {}
};
int func(int unused, ...)
{
	va_list args;
	va_start(args, unused);
	return va_arg(args, int);
}


typedef void(Fun::*pf)(int, int);

typedef void(Fun_son::*pf_son)(int, int);

int testfun()
{
	pFun oldFun = &fun1;
	pFun newFun = &fun2;
	/*
	pf oldFun = &Fun::fun;
	pf newFun = &Fun::fun_v1;

	int old = func(0, &Fun::fun);
	int ne = func(0, &Fun::fun_v1);

	pf_son of1 = &Fun_son::fun;
	pf_son of2 = &Fun_son::fun_v1;

	int f1 = func(0, &Fun_son::fun);
	int f2 = func(0, &Fun_son::fun_v1);
	*/

	pFun baseFun = &fun_base;

	fun1();

	LPCVOID pfun1 = (LPCVOID)((INT64)baseFun + (0x140001140 - 0x140001023));
	LPCVOID pfun2 = (LPCVOID)((INT64)baseFun + (0x14000115E - 0x140001023));

	WriteJMP_x86(pfun1, pfun2);

	fun1();
	fun1();

	return 0;
}



// ScanAddress ����x86(ɨ�跶Χ���С) x64��Χ̫��
uintptr_t hanshu_dizhi; //��¼�������Ӧ�ĵ�ַ
uintptr_t ScanAddress(HANDLE process, char *markCode, int nOffset, unsigned long dwReadLen = 4, uintptr_t StartAddr = 0x00400000, uintptr_t EndAddr = 0x7FFFFFFF, int InstructionLen = 0)
{
	//************���������룬ת�����ֽ�*****************
	if (strlen(markCode) % 2 != 0) return 0;
	//�����볤��
	int len = strlen(markCode) / 2;  //��ȡ������ֽ���

	//��������ת����byte�� ������m_code ��
	BYTE *m_code = new BYTE[len];
	for (int i = 0; i < len; i++)
	{
		//��������ɵ����ַ���һ�ֻ����������͡�
		char c[] = { markCode[i * 2], markCode[i * 2 + 1], '\0' };
		//������nptr�ַ������ݲ���base��ת���ɳ�������
		m_code[i] = (BYTE)::strtol(c, NULL, 16);
	}
	//ÿ�ζ�ȡ��Ϸ�ڴ���Ŀ�Ĵ�С
	const DWORD pageSize = 4096;

	// ����������
	//ÿҳ��ȡ4096���ֽ�
	BYTE *page = new BYTE[pageSize];
	uintptr_t tmpAddr = StartAddr;
	//�����������һ�����ȵı�ʶ
	int compare_one = 0;

	while (tmpAddr <= EndAddr)
	{
		::ReadProcessMemory(process, (LPCVOID)tmpAddr, page, pageSize, 0); //��ȡ0x400000���ڴ����ݣ�������page������ΪpageSize

		//�ڸ�ҳ�в���������
		for (int i = 0; i < pageSize; i++)
		{
			if (m_code[0] == page[i])//��һ���ֽ���������ĵ�һ���ֽ���ͬ��������
			{
				for (int j = 0; j<len - 1; j++)
				{
					if (m_code[j + 1] == page[i + j + 1])//�Ƚ�ÿһ���ֽڵĴ�С������ͬ���˳�
					{
						compare_one++;
					}
					else
					{
						compare_one = 0;
						break;
					}//����¸��Աȵ��ֽڲ���ȣ����˳���������Դ������
				}

				if ((compare_one + 1) == len)
				{
					// �ҵ������봦��
					//��ֵʱҪ����ʼֵ�������ͻ
					uintptr_t dwAddr = tmpAddr + i + nOffset;
					uintptr_t ullRet = 0;
					::ReadProcessMemory(process, (void*)dwAddr, &ullRet, dwReadLen, 0);
					//cout<<dwAddr<<endl;
					//�����dwAddr�Ѿ���Ӧ�����������ĵ�ַ
					//��ַ�����Ҳ��10����    ��Ҫת��Ϊ16���� 
					hanshu_dizhi = dwAddr;//��¼��ַ
					if (InstructionLen)
					{
						ullRet += dwAddr + dwReadLen;
					}

					return ullRet;
				}
			}
		}

		tmpAddr = tmpAddr + pageSize - len;//��һҳ����Ҫ��ǰһҳ��󳤶�len ��ʼ���ң�������ҳ�����м�������������������
	}

	return 0;
}

void getFuncAddr()
{
	/*
	HWND hWnd;
	hWnd = FindWindow(NULL, "Tutorial-i386");

	DWORD PID;
	GetWindowThreadProcessId(hWnd, &PID);

	HANDLE lsProcess;
	lsProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	*/
	HMODULE oldhModule = GetModuleHandleA(NULL);
	LPCVOID pfun1 = nullptr;
	LPCVOID pfun2 = nullptr;
	if (oldhModule)
	{
		pfun1 = GetProcAddress(oldhModule, "?fun_v1@Fun_son@@UAEXHH@Z");
		pfun2 = GetProcAddress(oldhModule, "?fun2@@YAXXZ");
	}
	pFun oldFun = &fun_base;

	HANDLE lsProcess = GetCurrentProcess();

	std::cout << ScanAddress(lsProcess, "E97A460000", 0) << std::endl;
	//2983AC0400008D45D4��Ӧ��ͼ��0x0042578F��ַ��������
	std::cout << hanshu_dizhi << std::endl;

	std::cout << "---------------------------" << std::endl;
}


int main()
{
	Fun* cA = new(std::nothrow) Fun_son;
	if (!cA)
		return 0;

	
	pFun baseFun = &fun_base;
	/*
	HMODULE oldhModule = GetModuleHandle(NULL);	// ��ȡ�����̾��
	LPCVOID baseFun = nullptr;
	if (oldhModule)
	{
		baseFun = GetProcAddress(oldhModule, "fun_base");	// ��Ҫ�������� ����ͨ��GetProcAddress��ú�����ַ
	}
	*/
	LPCVOID pOldfun = nullptr;
	LPCVOID pOldfun_v1 = nullptr;
	//HANDLE lsProcess = GetCurrentProcess();
	
	//ScanAddress(lsProcess, "E9B3140000", 0);
	//std::cout << hanshu_dizhi << std::endl;
	//pOldfun = (LPCVOID)hanshu_dizhi;
	
	// ���ݷ�����ļ�����fun_base������ڵ�ַ ���ҳ���Ҫ�滻�ĺ�����ڵ�ַ ����2��֮������ƫ�� Ȼ�����fun_baseʵ�������ڴ��ַ�����Ҫ�滻�ĺ���ʵ�������ڴ��ַ
	pOldfun = (LPCVOID)((INT64)baseFun + (0x12E0 - 0x1AA0));	

	//ScanAddress(lsProcess, "E9F4150000", 0);
	//std::cout << hanshu_dizhi << std::endl;
	//pOldfun_v1 = (LPCVOID)hanshu_dizhi;

	pOldfun_v1 = (LPCVOID)((INT64)baseFun + (0x12C0 - 0x1AA0));
	/*
	HMODULE oldhModule = GetModuleHandleA(NULL);	// ��ȡ�����̾��
	if (oldhModule)
	{
		pOldfun = GetProcAddress(oldhModule, "?fun@Fun_son@@UAEXHH@Z");	// ��Ҫ�������� ����ͨ��GetProcAddress��ú�����ַ
		pOldfun_v1 = GetProcAddress(oldhModule, "?fun_v1@Fun_son@@UAEXHH@Z");
	}
	*/
	cA->fun(10, 12);
	cA->fun_v1(10, 10);

	HMODULE hModule = LoadLibrary("fun.dll");
	int error = GetLastError();
	if (hModule)
	{
		// ��Ҫ���õ������� ����ͨ��GetProcAddress��ú�����ַ ������������ dumpbin /exports ��̬��(ִ���ļ�) ���鿴  (c++�����������ݺ�����������������������ʵ�ʵĺ�������)
		LPCVOID pNewfun = GetProcAddress(hModule, "?fun@Fun_son@@UAEXHH@Z");
		LPCVOID pNewfun_v1 = GetProcAddress(hModule, "?fun_v1@Fun_son@@UAEXHH@Z");

		WriteJMP_x86(pOldfun, pNewfun);
		WriteJMP_x86(pOldfun_v1, pNewfun_v1);
		//WriteJMP_x64(pOldfun, pNewfun);
		//WriteJMP_x64(pOldfun_v1, pNewfun_v1);	
	}
	cA->fun(10, 12);
	std::cout << cA->getn() << std::endl;
	cA->fun_v1(10, 10);
	std::cout << cA->getn() << std::endl;
	fun_base();
	error = GetLastError();

	FreeLibrary(hModule);
	return 0;
}
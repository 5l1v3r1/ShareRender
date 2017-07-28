//�۲�CPU��Ϣ����
#pragma once
#include "watcher_defines.h"

class CpuWatch{
public:
	CpuWatch(void);
	CpuWatch(char *procName);	//���캯��
	~CpuWatch(void);
	//ʱ��ת������
	static int64_t FileTimeToDouble(FILETIME &t);

	//����CPU���������
	double GetProcessCpuUtilization();	//��ȡ����CPU������
	double GetProcessCpuUtilization(HANDLE proc);

	//ϵͳCPU���������
	double GetSysCpuUtilization();	//��ȡϵͳCPU������
	unsigned int GetProcessorNum();	//��ȡCPU����
	int64_t GetCpuFreq();	//��ȡCPUƵ��
	//void GetSysCpuUtilizationLoop(double &result,int sleepTime);	//�����Եػ�ȡϵͳCPU������

	//���̲������
	HANDLE getProcH(char *procName);	//���ݽ�������ȡ���̺�
	HANDLE GetProcH();	
	inline void SetHandle(HANDLE handle){ procHandle = handle; }
	void SetProcName(char *input);			//���ý�����

private:
	static int64_t last_sys_kernel;
	static int64_t last_sys_user;
	static int64_t last_sys_idle;
	
	static int64_t last_time;
	static int64_t last_sys_time;
	static unsigned int processor_num;	//CPU����
	char processName[MAXSIZE];	//������
	HANDLE procHandle;
};
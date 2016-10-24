//�۲�CPU��Ϣ����
#pragma once
#include "WatcherDefines.h"

namespace cg{
	namespace core{

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
			int64_t last_sys_kernel;
			int64_t last_sys_user;
			int64_t last_sys_idle;

			int64_t last_time;
			int64_t last_sys_time;
			static unsigned int processor_num;	//CPU����
			char processName[MAXSIZE];	//������
			HANDLE procHandle;
		};

	}
}
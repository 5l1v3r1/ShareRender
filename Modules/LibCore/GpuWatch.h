#pragma once
#ifndef UTILITY_H
#define UTILITY_H
#include "WatcherDefines.h"



////����
class GpuInterface
{
public:
	virtual int GetGpuUsage(){	return -1;	};
	virtual int GetGpuTemp(){	return -1;	};
	virtual bool InitInterface(){return true;}
	~GpuInterface(){}
};
namespace cg{
	namespace core{

		//��ȡ�Կ������ʺ��¶���Ϣ����
		class GpuWatch
		{
		public:
			
			~GpuWatch(void);
			//��ʼ���Կ�
			bool InitGpuInterface(void);
			int GetGpuUsage();	//��ȡGPU������
			int GetGpuTemp();	//��ȡGPU�¶�
			void GetGpuInformation(char *buf,int size);	//��ȡGPU������Ϣ
			int gpuUtilization , gpuTemp , graNum;	//gpu�����ʺ��¶�,�Կ���Ŀ
			/*static GpuInterface* pInterface;*/
			std::string graInfo;	//�Կ���Ϣ
			void ChangeToLower(std::string &str);
			int type;

			static GpuWatch * GetGpuWatch(){
				if(!gpuWatch){
					gpuWatch = new GpuWatch();
				}
				return gpuWatch;
			}


		private:
			bool isInit;	//�Կ��Ƿ��Ѿ���ʼ��
			GpuInterface * gpuInterface;
			GpuWatch(void);

			static GpuWatch * gpuWatch;
		};


		//AMD�Կ���ص���,�̳�GpuInterface��
		class AMDInterface: public GpuInterface
		{
		public:
			AMDInterface(void);	//���캯��
			virtual ~AMDInterface();
			static void* _stdcall ADLMainMemoryAlloc(int size);	//����ռ�
			static void _stdcall ADLMainMemoryFree(void **buf);	//�ͷſռ�

			virtual int GetGpuUsage();	//��ȡGPU����
			virtual int GetGpuTemp();	//��ȡGPU�¶�
			virtual bool InitInterface();

			//���庯��ָ��
			static ADL_MAIN_CONTROL_CREATE  AdlMainControlCreate;
			static ADL_MAIN_CONTROL_REFRESH AdlMainControlRefresh;
			static ADL_OVERDRIVE5_TEMPERATURE_GET AdlOverDrive5TemperatureGet;
			static ADL_OVERDRIVE5_CURRENTACTIVITY_GET AdlOverDrive5CurrentActivityGet;

		private:
			bool InitAdlApi();	//��ʼ��API
			bool isInit;	//�Ƿ��Ѿ���ʼ��
		};

		//Nvidia�Կ���ص��࣬�̳�GpuInterface��
		class NvidiaInterface: public GpuInterface
		{
		public:
			NvidiaInterface(void);	//���캯��
			virtual ~NvidiaInterface();
			virtual int GetGpuUsage();	//��ȡGPU������
			virtual int GetGpuTemp();	//��ȡGPU�¶�
			virtual bool InitInterface();
			
		private:
			bool InitNvApi();	//��ʼ��N��
			bool isInit;
			NvPhysicalGpuHandle phys;
		};

		class NvApiInterface: public GpuInterface{
		public:
			NvApiInterface();
			virtual ~NvApiInterface();
			virtual int GetGpuUsage();
			virtual int GetGpuTemp();
			virtual bool InitInterface();
		private:
			bool isInit;
			int data0;

			// function pointer
			void * call0;
			void * call1;
			void * call2;

			int buffer[1024];
		};

	}
}

#endif

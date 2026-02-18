#pragma once
#ifndef GEOM_IPC_H
#define GEOM_IPC_H

/**
 * @file GeomIPC.h
 * @brief 进程间通信协议（SimulationTool <-> GeomProcessor）
 *
 * 通信方式：Qt QSharedMemory，key = "GeomIPC"
 *
 * 流程：
 *   1. SimulationTool 将当前几何写成 STEP 临时文件
 *   2. 将路径写入共享内存，cmd = CMD_SEND_GEOM
 *   3. GeomProcessor 检测到 CMD_SEND_GEOM 后读取文件，处理完毕
 *   4. GeomProcessor 将结果写回 STEP 临时文件，cmd = CMD_RESULT_READY
 *   5. SimulationTool 可选择读取结果并刷新视图
 */

#include <cstdint>
#include <cstring>

static const int GEOM_IPC_MAGIC   = 0xCAD0B000;
static const int GEOM_IPC_VERSION = 1;

/// 共享内存 key（两端一致）
static const char GEOM_IPC_KEY[] = "GeomIPC_v1";

/// 共享内存总大小（1 KB 足够放路径和状态）
static const int GEOM_IPC_SIZE = 2048;

/// 命令码
enum GeomCmd : uint32_t
{
    CMD_IDLE         = 0,   ///< 空闲 / 已读取
    CMD_SEND_GEOM    = 1,   ///< SimulationTool → GeomProcessor：新几何已写入 geomFilePath
    CMD_PROCESSING   = 2,   ///< GeomProcessor：正在处理
    CMD_RESULT_READY = 3,   ///< GeomProcessor → SimulationTool：结果已写入 resultFilePath
    CMD_ERROR        = 4,   ///< 发生错误，见 errorMsg
};

#pragma pack(push, 1)
struct GeomIPCBlock
{
    uint32_t magic;             ///< = GEOM_IPC_MAGIC
    uint32_t version;           ///< = GEOM_IPC_VERSION
    uint32_t cmd;               ///< GeomCmd
    uint32_t seqNo;             ///< 序列号，每次发送递增
    char     geomFilePath[512]; ///< SimulationTool → GeomProcessor STEP 文件路径
    char     resultFilePath[512];///< GeomProcessor → SimulationTool 结果 STEP 路径
    char     errorMsg[256];     ///< 错误信息（CMD_ERROR 时有效）
    char     _pad[256];         ///< reserved

    GeomIPCBlock()
    {
        memset(this, 0, sizeof(*this));
        magic   = GEOM_IPC_MAGIC;
        version = GEOM_IPC_VERSION;
        cmd     = CMD_IDLE;
        seqNo   = 0;
    }
};
#pragma pack(pop)

static_assert(sizeof(GeomIPCBlock) <= GEOM_IPC_SIZE,
              "GeomIPCBlock exceeds GEOM_IPC_SIZE");

#endif // GEOM_IPC_H

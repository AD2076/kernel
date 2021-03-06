
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "MtcIntrusion.h"
#include "MtcDebug.h"

#include "MtcComm.h"

#include "UsimPsInterface.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define    THIS_FILE_ID        PS_FILE_ID_MTC_INTRUSION_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/* 干扰控制策略查询表 */
const MTC_INTRUSION_RULING_STRATEGY_STRU g_astMtcIntrusionRulingStrategyTbl[] =
{
    {MTC_INTRUSION_GSM_BAND_NONE,       MTC_INTRUSION_TL_BAND_NONE,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB3B39},

    {MTC_INTRUSION_GSM_BAND_B31,        MTC_INTRUSION_TL_BAND_NONE,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB39},

    {MTC_INTRUSION_GSM_BAND_B31_OTHER,  MTC_INTRUSION_TL_BAND_NONE,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB39},

    {MTC_INTRUSION_GSM_BAND_B32_OTHER,  MTC_INTRUSION_TL_BAND_NONE,
     MTC_CS_NO_SERVICE,                 MTC_IntrusionEnableB39},

    {MTC_INTRUSION_GSM_BAND_B3_OTHER,   MTC_INTRUSION_TL_BAND_NONE,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB39},

    {MTC_INTRUSION_GSM_BAND_OTHER,      MTC_INTRUSION_TL_BAND_NONE,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB3B39},

    {MTC_INTRUSION_GSM_BAND_NONE,       MTC_INTRUSION_TL_BAND_WITH_B39,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB3},

    {MTC_INTRUSION_GSM_BAND_B32,        MTC_INTRUSION_TL_BAND_WITH_B39,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionDisableB39},

    {MTC_INTRUSION_GSM_BAND_B3,         MTC_INTRUSION_TL_BAND_WITH_B39,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionDisableB39},

    {MTC_INTRUSION_GSM_BAND_B32_OTHER,  MTC_INTRUSION_TL_BAND_WITH_B39,
     MTC_CS_IN_SERVICE,                 MTC_IntrusionDisableB39},

    {MTC_INTRUSION_GSM_BAND_B32_OTHER,  MTC_INTRUSION_TL_BAND_WITH_B39,
     MTC_CS_NO_SERVICE,                 MTC_IntrusionDisableB3},

    {MTC_INTRUSION_GSM_BAND_B3_OTHER,   MTC_INTRUSION_TL_BAND_WITH_B39,
     MTC_CS_IN_SERVICE,                 MTC_IntrusionDisableB39},

    {MTC_INTRUSION_GSM_BAND_B3_OTHER,   MTC_INTRUSION_TL_BAND_WITH_B39,
     MTC_CS_NO_SERVICE,                 MTC_IntrusionDisableB3},

    {MTC_INTRUSION_GSM_BAND_NONE,       MTC_INTRUSION_TL_BAND_WITHOUT_B39,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB3B39},

    {MTC_INTRUSION_GSM_BAND_B31,        MTC_INTRUSION_TL_BAND_WITHOUT_B39,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB39},

    {MTC_INTRUSION_GSM_BAND_B31_OTHER,  MTC_INTRUSION_TL_BAND_WITHOUT_B39,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB39},

    {MTC_INTRUSION_GSM_BAND_B32_OTHER,  MTC_INTRUSION_TL_BAND_WITHOUT_B39,
     MTC_CS_NO_SERVICE,                 MTC_IntrusionEnableB39},

    {MTC_INTRUSION_GSM_BAND_B3_OTHER,   MTC_INTRUSION_TL_BAND_WITHOUT_B39,
     MTC_CS_NO_SERVICE,                 MTC_IntrusionEnableB39},

    {MTC_INTRUSION_GSM_BAND_OTHER,      MTC_INTRUSION_TL_BAND_WITHOUT_B39,
     MTC_CS_SERVICE_STATE_BUTT,         MTC_IntrusionEnableB3B39},
};

/* 根据GSM的频段信息，查询对应的频段状态 */
const MTC_INTRUSION_GSM_BAND_STATE_STRU g_stMtcIntrusionGsmBandStateTbl[] =
{
    {VOS_FALSE, VOS_FALSE, VOS_FALSE, MTC_INTRUSION_GSM_BAND_NONE},
    {VOS_TRUE,  VOS_FALSE, VOS_FALSE, MTC_INTRUSION_GSM_BAND_B31},
    {VOS_FALSE, VOS_TRUE,  VOS_FALSE, MTC_INTRUSION_GSM_BAND_B32},
    {VOS_FALSE, VOS_FALSE, VOS_TRUE,  MTC_INTRUSION_GSM_BAND_OTHER},
    {VOS_TRUE,  VOS_TRUE,  VOS_FALSE, MTC_INTRUSION_GSM_BAND_B3},
    {VOS_TRUE,  VOS_FALSE, VOS_TRUE,  MTC_INTRUSION_GSM_BAND_B31_OTHER},
    {VOS_FALSE, VOS_TRUE,  VOS_TRUE,  MTC_INTRUSION_GSM_BAND_B32_OTHER},
    {VOS_TRUE,  VOS_TRUE,  VOS_TRUE,  MTC_INTRUSION_GSM_BAND_B3_OTHER},
};


/*****************************************************************************
  3 函数实现
*****************************************************************************/

VOS_VOID MTC_SndRrcIntrusionActionSetReq(
    VOS_UINT32                          ulRecvPid,
    PS_BOOL_ENUM_UINT8                  enEnableFlag
)
{
    VOS_UINT16                          ulLenth;
    MTC_RRC_INTRUSION_ACTION_SET_REQ_STRU   *pstMtcRrcActionReq = VOS_NULL_PTR;

    ulLenth             = sizeof(MTC_RRC_INTRUSION_ACTION_SET_REQ_STRU)
                            - VOS_MSG_HEAD_LENGTH;

    /* 分配消息空间 */
    pstMtcRrcActionReq  = (MTC_RRC_INTRUSION_ACTION_SET_REQ_STRU *)PS_ALLOC_MSG(
                                    UEPS_PID_MTC,
                                    ulLenth);

    if (VOS_NULL_PTR == pstMtcRrcActionReq)
    {
        MTC_ERROR_LOG("MTC_SndRrcIntrusionActionSetReq: alloc msg fail.");
        return;
    }

    /* 清消息空间 */
    PS_MEM_SET((VOS_UINT8*)pstMtcRrcActionReq + VOS_MSG_HEAD_LENGTH, 0, ulLenth);

    /* 填充消息 */
    pstMtcRrcActionReq->stMsgHeader.ulReceiverPid   = ulRecvPid;
    pstMtcRrcActionReq->stMsgHeader.ulMsgName       = ID_MTC_RRC_INTRUSION_ACTION_SET_REQ;
    pstMtcRrcActionReq->enActionFlag                = enEnableFlag;

    /* 发送消息 */
    if (VOS_OK != PS_SEND_MSG(UEPS_PID_MTC, pstMtcRrcActionReq))
    {
        MTC_ERROR1_LOG("MTC_SndRrcIntrusionActionSetReq: Snd Msg err. Rec Pid ", ulRecvPid);
        return;
    }

    MTC_DEBUG_IntrusionSendActionReq(ulRecvPid);

    return;
}


VOS_VOID MTC_SndIntrusionActionSetReq(
    MODEM_ID_ENUM_UINT16                enModemId,
    PS_BOOL_ENUM_UINT8                  enEnableFlg
)
{
    if (MODEM_ID_0 == enModemId)
    {
        /* 给Modem0的接入层发送消息*/
#if (FEATURE_ON == FEATURE_UE_MODE_TDS)
        /* 判断当前接入技术是否支持TDS */
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_0, MTC_RATMODE_TDS))
        {
            MTC_SndRrcIntrusionActionSetReq(TPS_PID_RRC, enEnableFlg);
        }
#endif
#if (FEATURE_ON == FEATURE_LTE)
        /* 判断当前接入技术是否支持LTE */
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_0, MTC_RATMODE_LTE))
        {
            MTC_SndRrcIntrusionActionSetReq(PS_PID_ERRC, enEnableFlg);
        }
#endif
    }
    else if (MODEM_ID_1 == enModemId)
    {
        /* 判断当前接入技术是否支持G模*/
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_1, MTC_RATMODE_GSM))
        {
            /* 给Modem1 GAS发送消息*/
            MTC_SndRrcIntrusionActionSetReq(I1_UEPS_PID_GAS, enEnableFlg);
        }
    }
    else
    {
        MTC_ERROR_LOG("MTC_SndIntrusionActionSetReq: Modem ID Err.");
    }

    return;

}
VOS_VOID MTC_SndRrcIntrusionBandSetReq(
    VOS_UINT32                          ulRecvPid,
    RRC_MTC_MS_BAND_INFO_STRU          *pstForbiddenBandInfo
)
{
    VOS_UINT16                              ulLenth;
    MODEM_ID_ENUM_UINT16                    enModemId;
    MTC_RRC_INTRUSION_BAND_SET_REQ_STRU    *pstBandSetReq = VOS_NULL_PTR;

    enModemId = VOS_GetModemIDFromPid(ulRecvPid);
    if (enModemId >= MODEM_ID_BUTT)
    {
        return;
    }

    /* 如果当前已经关机，则停止发送信息 */
    if (MTC_MODEM_POWER_OFF == MTC_GetModemPowerState(enModemId))
    {
        return;
    }

    /* 分配消息空间 */
    ulLenth         = sizeof(MTC_RRC_INTRUSION_BAND_SET_REQ_STRU);
    pstBandSetReq   = (MTC_RRC_INTRUSION_BAND_SET_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(
                                                                UEPS_PID_MTC,
                                                                ulLenth);

    if (VOS_NULL_PTR == pstBandSetReq)
    {
        MTC_ERROR_LOG("MTC_SndRrcIntrusionActionSetReq: Alloc Msg Fail!");
        return;
    }

    /* 填充消息 */
    pstBandSetReq->stMsgHeader.ulReceiverPid    = ulRecvPid;
    pstBandSetReq->stMsgHeader.ulMsgName        = ID_MTC_RRC_INTRUSION_BAND_SET_REQ;

    PS_MEM_CPY(&pstBandSetReq->stForbiddenBandInfo,
               pstForbiddenBandInfo,
               sizeof(RRC_MTC_MS_BAND_INFO_STRU));

    /* 发送消息 */
    if (VOS_OK != PS_SEND_MSG(UEPS_PID_MTC, pstBandSetReq))
    {
        MTC_ERROR1_LOG("MTC_SndRrcIntrusionActionSetReq: Snd Msg Err. ReceiverPid: ", ulRecvPid);
        return;
    }

    /* 记录可维可测 */
    MTC_DEBUG_IntrusionSendSetBandReq(ulRecvPid);

    return;

}
VOS_VOID MTC_SndTdsLteIntrusionBandSetReq(
    RRC_MTC_MS_BAND_INFO_STRU          *pstForbiddenBandInfo
)
{
    /* 判断当前主模状态, 并分别给TDS及LTE接入层发送禁用B39指示 */
    if (MTC_RATMODE_LTE == MTC_GetModemRatMode(MODEM_ID_0))
    {
#if (FEATURE_ON == FEATURE_LTE)
        /* 判断当前接入技术是否支持LTE */
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_0, MTC_RATMODE_LTE))
        {
            MTC_SndRrcIntrusionBandSetReq(PS_PID_ERRC, pstForbiddenBandInfo);
        }
#endif
#if (FEATURE_ON == FEATURE_UE_MODE_TDS)
        /* 判断当前接入技术是否支持TDS */
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_0, MTC_RATMODE_TDS))
        {
            MTC_SndRrcIntrusionBandSetReq(TPS_PID_RRC, pstForbiddenBandInfo);
        }
#endif
    }
    else
    {
#if (FEATURE_ON == FEATURE_UE_MODE_TDS)
        /* 判断当前接入技术是否支持TDS */
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_0, MTC_RATMODE_TDS))
        {
            MTC_SndRrcIntrusionBandSetReq(TPS_PID_RRC, pstForbiddenBandInfo);
        }
#endif
#if (FEATURE_ON == FEATURE_LTE)
        /* 判断当前接入技术是否支持LTE */
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_0, MTC_RATMODE_LTE))
        {
            MTC_SndRrcIntrusionBandSetReq(PS_PID_ERRC, pstForbiddenBandInfo);
        }
#endif
    }

    return;
}
VOS_VOID MTC_IntrusionDisableB39(VOS_VOID)
{
    RRC_MTC_MS_BAND_INFO_STRU           stForbiddenBandInfo;

    PS_MEM_SET(&stForbiddenBandInfo, 0, sizeof(stForbiddenBandInfo));

    stForbiddenBandInfo.aulBandInfo[1] |= 1 << MTC_BAND_B39_BIT;

    MTC_SndTdsLteIntrusionBandSetReq(&stForbiddenBandInfo);

    return;
}


VOS_VOID MTC_IntrusionDisableB3(VOS_VOID)
{
    RRC_MTC_MS_BAND_INFO_STRU           stForbiddenBandInfo;

    PS_MEM_SET(&stForbiddenBandInfo, 0, sizeof(stForbiddenBandInfo));

    stForbiddenBandInfo.aulBandInfo[0] |= 1 << MTC_BAND_B3_BIT;

    /* 给GAS接入层发送禁用B3指示 */
    if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_1, MTC_RATMODE_GSM))
    {
        MTC_SndRrcIntrusionBandSetReq(I1_UEPS_PID_GAS, &stForbiddenBandInfo);
    }

    return;
}
VOS_VOID MTC_IntrusionEnableB39(VOS_VOID)
{
    RRC_MTC_MS_BAND_INFO_STRU           stForbiddenBandInfo;

    PS_MEM_SET(&stForbiddenBandInfo, 0, sizeof(stForbiddenBandInfo));

    MTC_SndTdsLteIntrusionBandSetReq(&stForbiddenBandInfo);

    return;
}


VOS_VOID MTC_IntrusionEnableB3(VOS_VOID)
{
    RRC_MTC_MS_BAND_INFO_STRU           stForbiddenBandInfo;

    PS_MEM_SET(&stForbiddenBandInfo, 0, sizeof(stForbiddenBandInfo));

    /* 给GAS接入层发送启用B3指示 */
    if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_1, MTC_RATMODE_GSM))
    {
        MTC_SndRrcIntrusionBandSetReq(I1_UEPS_PID_GAS, &stForbiddenBandInfo);
    }

    return;
}
VOS_VOID MTC_IntrusionEnableB3B39(VOS_VOID)
{
    /* 调用启用B3流程 */
    MTC_IntrusionEnableB3();

    /* 调用启用B39流程 */
    MTC_IntrusionEnableB39();

    return;
}


VOS_VOID MTC_ProcIntrusionStrategy(VOS_VOID)
{
    MTC_MODEM_POWER_STATE_ENUM_UINT8        enModem0PowerState;
    MTC_MODEM_POWER_STATE_ENUM_UINT8        enModem1PowerState;
    VOS_UINT32                              ulLoop;
    VOS_UINT32                              ulLineNum;
    MTC_INTRUSION_GSM_BAND_STATE_ENUM_UINT8 enGsmBandState;
    MTC_INTRUSION_TL_BAND_STATE_ENUM_UINT8  enTLBandState;
    MTC_CS_SERVICE_STATE_ENUM_UINT8         enCsSrvState;
    FUNC_VOID                               pFunc           = VOS_NULL_PTR;

    /* 获取Modem0与Modem1的开机状态，只有在同时开机的情况下才进行裁决 */
    enModem0PowerState  = MTC_GetModemPowerState(MODEM_ID_0);
    enModem1PowerState  = MTC_GetModemPowerState(MODEM_ID_1);
    if ( (MTC_MODEM_POWER_ON != enModem0PowerState)
      || (MTC_MODEM_POWER_ON != enModem1PowerState) )
    {
        return;
    }

    /* 获取本地存储的频段信息，查询裁决策略表，执行对应操作 */
    ulLineNum       = sizeof(g_astMtcIntrusionRulingStrategyTbl) / sizeof(MTC_INTRUSION_RULING_STRATEGY_STRU);
    enGsmBandState  = MTC_GetModem1GsmBandState();
    enTLBandState   = MTC_GetModem0TLBandState();
    enCsSrvState    = MTC_GetModemCsSrvExistFlg(MODEM_ID_1);

    for (ulLoop = 0; ulLoop < ulLineNum; ulLoop++)
    {
        if (g_astMtcIntrusionRulingStrategyTbl[ulLoop].enGsmBandState != enGsmBandState)
        {
            continue;
        }

        if (g_astMtcIntrusionRulingStrategyTbl[ulLoop].enTLBandState != enTLBandState)
        {
            continue;
        }

        if (MTC_CS_SERVICE_STATE_BUTT != g_astMtcIntrusionRulingStrategyTbl[ulLoop].enCsSrvState)
        {
            if (enCsSrvState != g_astMtcIntrusionRulingStrategyTbl[ulLoop].enCsSrvState)
            {
                continue;
            }
        }

        pFunc = g_astMtcIntrusionRulingStrategyTbl[ulLoop].pFunc;

        break;
    }

    if (VOS_NULL_PTR != pFunc)
    {
        pFunc();
    }

    return;
}


VOS_VOID MTC_IntrusionModemPowerStateChange(
    MODEM_ID_ENUM_UINT16                enModemId,
    MTC_MODEM_POWER_STATE_ENUM_UINT8    enPowerState
)
{
    if (MTC_MODEM_POWER_OFF == enPowerState)
    {
        if (MODEM_ID_0 == enModemId)
        {
            if (MTC_MODEM_POWER_ON == MTC_GetModemPowerState(MODEM_ID_1))
            {
                /* 给Modem1的GAS发送启用B3指示 */
                MTC_IntrusionEnableB3();
            }
        }
        else
        {
            if (MTC_MODEM_POWER_ON == MTC_GetModemPowerState(MODEM_ID_0))
            {
                /* 给Modem0的TD&LTE发送启用B39指示 */
                MTC_IntrusionEnableB39();
            }
        }
    }

    return;
}


MTC_INTRUSION_GSM_BAND_STATE_ENUM_UINT8 MTC_GetModem1GsmBandState(VOS_VOID)
{
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulLineNum;
    VOS_UINT8                           ucBand31;
    VOS_UINT8                           ucBand32;
    VOS_UINT8                           ucBandOther;
    RRC_MTC_MS_BAND_INFO_STRU          *pulBandInfo = VOS_NULL_PTR;
    MTC_INTRUSION_GSM_BAND_STATE_ENUM_UINT8 enGsmBandState;

    pulBandInfo = MTC_GetModemCurBandInfo(MODEM_ID_1);

    /* 通过读取Bit位获取当前Band状态 */
    if (0 != (pulBandInfo->aulBandInfo[0] & (1 << MTC_BAND_B31_BIT)))
    {
        ucBand31 = VOS_TRUE;
    }
    else
    {
        ucBand31 = VOS_FALSE;
    }

    if (0 != (pulBandInfo->aulBandInfo[0] & ((VOS_UINT32)1 << MTC_BAND_B32_BIT)))
    {
        ucBand32 = VOS_TRUE;
    }
    else
    {
        ucBand32 = VOS_FALSE;
    }

    if ( (0 != (pulBandInfo->aulBandInfo[0] & MTC_BAND_WITHOUT_B3))
      || (0 != pulBandInfo->aulBandInfo[1]) )
    {
        ucBandOther = VOS_TRUE;
    }
    else
    {
        ucBandOther = VOS_FALSE;
    }

    /* 通过查表获取当前GSM频段状态 */
    enGsmBandState  = MTC_INTRUSION_GSM_BAND_STATE_BUTT;
    ulLineNum       = sizeof(g_stMtcIntrusionGsmBandStateTbl) / sizeof(MTC_INTRUSION_GSM_BAND_STATE_STRU);

    for (ulLoop = 0; ulLoop < ulLineNum; ulLoop++)
    {
        if (g_stMtcIntrusionGsmBandStateTbl[ulLoop].ucBand31 != ucBand31)
        {
            continue;
        }

        if (g_stMtcIntrusionGsmBandStateTbl[ulLoop].ucBand32 != ucBand32)
        {
            continue;
        }

        if (g_stMtcIntrusionGsmBandStateTbl[ulLoop].ucBandOther != ucBandOther)
        {
            continue;
        }

        enGsmBandState = g_stMtcIntrusionGsmBandStateTbl[ulLoop].enGsmBandState;

        break;
    }

    return enGsmBandState;
}


MTC_INTRUSION_TL_BAND_STATE_ENUM_UINT8 MTC_GetModem0TLBandState(VOS_VOID)
{
    RRC_MTC_MS_BAND_INFO_STRU          *pulBandInfo = VOS_NULL_PTR;

    pulBandInfo = MTC_GetModemCurBandInfo(MODEM_ID_0);

    /* 通过读取Bit位获取当前Band状态 */
    if ( (0 == pulBandInfo->aulBandInfo[0])
      && (0 == pulBandInfo->aulBandInfo[1]) )
    {
        return MTC_INTRUSION_TL_BAND_NONE;
    }
    else if (0 != (pulBandInfo->aulBandInfo[1] & (1 << MTC_BAND_B39_BIT)))
    {
        return MTC_INTRUSION_TL_BAND_WITH_B39;
    }
    else
    {
        return MTC_INTRUSION_TL_BAND_WITHOUT_B39;
    }

}
VOS_VOID MTC_RcvIntrusionActionSetCnf(VOS_VOID * pMsg)
{
    RRC_MTC_RESULT_CNF_STRU            *pstResultCnf;
    VOS_UINT32                          ulSendPid;

    pstResultCnf = (RRC_MTC_RESULT_CNF_STRU*)pMsg;
    ulSendPid    = pstResultCnf->stMsgHeader.ulSenderPid;

    /* 如果回复失败，打印提示 */
    if (MTC_RRC_RESULT_NO_ERROR != pstResultCnf->enResult)
    {
        MTC_ERROR2_LOG("MTC_RcvModem1GasIntrusionActionSetCnf: Action fail.", ulSendPid, pstResultCnf->enResult);
    }

    MTC_DEBUG_IntrusionRcvActionCnf(ulSendPid);

    return;
}
VOS_VOID MTC_RcvIntrusionBandSetCnf(VOS_VOID * pMsg)
{
    RRC_MTC_RESULT_CNF_STRU            *pstResultCnf;
    VOS_UINT32                          ulSendPid;

    pstResultCnf = (RRC_MTC_RESULT_CNF_STRU*)pMsg;
    ulSendPid    = pstResultCnf->stMsgHeader.ulSenderPid;

    /* 如果回复失败，打印提示 */
    if (MTC_RRC_RESULT_NO_ERROR != pstResultCnf->enResult)
    {
        MTC_ERROR2_LOG("MTC_RcvIntrusionBandSetCnf: Set Band fail.", ulSendPid, pstResultCnf->enResult);
    }

    MTC_DEBUG_IntrusionRcvSetBandCnf(ulSendPid);

    return;
}
VOS_VOID MTC_RcvIntrusionBandInfoInd(VOS_VOID * pMsg)
{
    MODEM_ID_ENUM_UINT16                    enModemId;
    MTC_MODEM_INFO_STRU                    *pstModemInfo;
    RRC_MTC_INTRUSION_BAND_INFO_IND_STRU   *pstModemBandInfo;
    MTC_CFG_ENUM_UINT8                      enIntrusionCfg;
    MTC_CFG_ENUM_UINT8                      enNotchCfg;
#if 0
    MTC_CFG_ENUM_UINT8                      enNarrowDcsCfg;
#endif
    pstModemBandInfo    = (RRC_MTC_INTRUSION_BAND_INFO_IND_STRU*)pMsg;
    enModemId           = VOS_GetModemIDFromPid(pstModemBandInfo->stMsgHeader.ulSenderPid);

    /* 检查ModemID有效性，并更新本地全局变量 */
    if (enModemId >= MODEM_ID_BUTT)
    {
        MTC_ERROR1_LOG("MTC_RcvIntrusionBandInfoInd: Err ModemId ", enModemId);
        return;
    }

    pstModemInfo = MTC_GetModemCtxAddr(enModemId);

    PS_MEM_CPY(&pstModemInfo->stCurBandInfo,
               &pstModemBandInfo->stCurBandInfo,
               sizeof(RRC_MTC_MS_BAND_INFO_STRU));

    PS_MEM_CPY(&pstModemInfo->stSerCellBandInfo,
               &pstModemBandInfo->stSerCellBandInfo,
               sizeof(RRC_MTC_MS_BAND_INFO_STRU));

    PS_MEM_CPY(&pstModemInfo->stServiceBandInfo,
               &pstModemBandInfo->stServiceBandInfo,
               sizeof(RRC_MTC_MS_BAND_INFO_STRU));

    PS_MEM_CPY(&pstModemInfo->stNCellBandInfo,
               &pstModemBandInfo->stNCellBandInfo,
               sizeof(RRC_MTC_MS_BAND_INFO_STRU));

    enIntrusionCfg  = MTC_GetIntrusionCfg();
    enNotchCfg      = MTC_GetNotchCfg();

    /* 将当前MTC上下文上报到SDT */
    MTC_DEBUG_TraceCtxInfo();

    if (MTC_CFG_ENABLE == enIntrusionCfg)
    {
        /* 更新频段信息后，触发干扰控制裁决 */
        MTC_ProcIntrusionStrategy();
    }

    if (MTC_CFG_ENABLE == enNotchCfg)
    {
        /* 更新频段信息后，触发notch控制裁决 */
        MTC_ProcNotchStrategy();
    }
#if 0
    /* 判断Narrow band dcs裁决是否使能 */
    enNarrowDcsCfg = MTC_GetNarrowBandDcsCfg();
    if (MTC_CFG_ENABLE == enNarrowDcsCfg)
    {
        /* 执行Narrow band dcs裁决 */
        MTC_ProcNarrowDcsStrategy();
    }
#endif
    /* TL射频控制开关开启时, 触发干扰控制裁决 */
    if ( (MTC_CFG_ENABLE == MTC_GetTlRfCtrlCfg())
      && (MODEM_ID_0 == enModemId) )
    {
        MTC_ProcB39B32Intrusion();
    }

#if (FEATURE_ON == FEATURE_LTE)
    /* Modem1上报频段信息，需要通知LRRC */
    if (MODEM_ID_1 == enModemId)
    {
        MTC_SndLrrcModem1InfoInd();
    }
#endif

    return;
}


PS_BOOL_ENUM_UINT8 MTC_CheckGsmWithB3(
    RRC_MTC_MS_BAND_INFO_STRU          *pulBandInfo
)
{
    /* 通过读取Bit位获取当前Band状态 */
    if ((0 != (pulBandInfo->aulBandInfo[0] & (VOS_UINT32)(1 << MTC_BAND_B31_BIT)))
     || (0 != (pulBandInfo->aulBandInfo[0] & (VOS_UINT32)((VOS_UINT32)1 << MTC_BAND_B32_BIT))))
    {
        return PS_TRUE;
    }
    else
    {
        return PS_FALSE;
    }
}
PS_BOOL_ENUM_UINT8 MTC_CheckTLWithB34B39(
    RRC_MTC_MS_BAND_INFO_STRU          *pulBandInfo
)
{
    if ((0 != (pulBandInfo->aulBandInfo[1] & (VOS_UINT32)(1 << MTC_BAND_B34_BIT)))
     || (0 != (pulBandInfo->aulBandInfo[1] & (VOS_UINT32)(1 << MTC_BAND_B39_BIT))))
    {
        return PS_TRUE;
    }
    else
    {
        return PS_FALSE;
    }
}


VOS_VOID MTC_ProcNotchStrategy(VOS_VOID)
{
    MTC_MODEM_POWER_STATE_ENUM_UINT8        enModem0PowerState;
    MTC_MODEM_POWER_STATE_ENUM_UINT8        enModem1PowerState;
    MTC_INTRUSION_GSM_BAND_STATE_ENUM_UINT8 enGsmBandState;
    MTC_INTRUSION_TL_BAND_STATE_ENUM_UINT8  enTLBandState;
    PS_BOOL_ENUM_UINT8                      enWithB3Flag;
    PS_BOOL_ENUM_UINT8                      enWithB34B39Flag;

    /* 获取Modem0与Modem1的开机状态 */
    enModem0PowerState  = MTC_GetModemPowerState(MODEM_ID_0);
    enModem1PowerState  = MTC_GetModemPowerState(MODEM_ID_1);
    if ( (MTC_MODEM_POWER_ON != enModem0PowerState)
      || (MTC_MODEM_POWER_ON != enModem1PowerState) )
    {
        /* 两个Modem未同时开机时需要复位Notch功能 */
        MTC_ChangeNotchState(PS_FALSE);
        return;
    }

    /* 两个Modem同时开机，但其中一个或两个Modem不存在任何频段，需要使能Notch */
    enGsmBandState  = MTC_GetModem1GsmBandState();
    enTLBandState   = MTC_GetModem0TLBandState();
    if ( (MTC_INTRUSION_GSM_BAND_NONE == enGsmBandState)
      || (MTC_INTRUSION_TL_BAND_NONE == enTLBandState) )
    {
        MTC_ChangeNotchState(PS_TRUE);
        return;
    }

    enWithB3Flag     = MTC_CheckGsmWithB3(MTC_GetModemCurBandInfo(MODEM_ID_1));
    enWithB34B39Flag = MTC_CheckTLWithB34B39(MTC_GetModemCurBandInfo(MODEM_ID_0));

    /* GSM B3和TL B34/B39同时存在时需要使能Notch，否则复位Notch */
    if (PS_TRUE == (enWithB3Flag & enWithB34B39Flag))
    {
        MTC_ChangeNotchState(PS_TRUE);
    }
    else
    {
        MTC_ChangeNotchState(PS_FALSE);
    }

    return;
}


VOS_VOID MTC_ChangeNotchState(
    PS_BOOL_ENUM_UINT8                  enEnableFlag
)
{
    /* 如果与当前状态一致则直接返回 */
    if (MTC_GetNotchEnalbeFlag() == enEnableFlag)
    {
        return;
    }

    /* 只有在该Modem开机时才下发裁决指示 */
    if (MTC_MODEM_POWER_ON == MTC_GetModemPowerState(MODEM_ID_1))
    {
        /* 给副卡GAS下发NOTCH裁决 */
        MTC_SndRrcNotchChannelInd(I1_UEPS_PID_GAS, enEnableFlag);
    }

    if (MTC_MODEM_POWER_ON == MTC_GetModemPowerState(MODEM_ID_0))
    {
#if (FEATURE_ON == FEATURE_UE_MODE_TDS)
        /* 判断当前接入技术是否支持TDS */
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_0, MTC_RATMODE_TDS))
        {
            /* 如果支持TDS模式，给主卡TDS RRC下发NOTCH裁决 */
            MTC_SndRrcNotchChannelInd(TPS_PID_RRC, enEnableFlag);
        }
#endif
#if (FEATURE_ON == FEATURE_LTE)
        /* 判断当前接入技术是否支持LTE */
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_0, MTC_RATMODE_LTE))
        {
            /* 如果支持LTE模式，给主卡LTE RRC下发NOTCH裁决 */
            MTC_SndRrcNotchChannelInd(PS_PID_ERRC, enEnableFlag);
        }
#endif
    }

    /* 记录Notch标志 */
    MTC_SetNotchEnalbeFlag(enEnableFlag);

    return;
}


VOS_VOID MTC_SndRrcNotchChannelInd(
    VOS_UINT32                          ulRecvPid,
    PS_BOOL_ENUM_UINT8                  enEnableFlag
)
{
    VOS_UINT16                          ulLenth;
    MTC_RRC_NOTCH_CHANNEL_IND_STRU     *pstNotchChannelInd = VOS_NULL_PTR;

    ulLenth = sizeof(MTC_RRC_NOTCH_CHANNEL_IND_STRU) - VOS_MSG_HEAD_LENGTH;

    /* 分配消息空间 */
    pstNotchChannelInd  = (MTC_RRC_NOTCH_CHANNEL_IND_STRU *)PS_ALLOC_MSG(
                                    UEPS_PID_MTC,
                                    ulLenth);

    if (VOS_NULL_PTR == pstNotchChannelInd)
    {
        MTC_ERROR_LOG("MTC_SndRrcNotchChannelInd: alloc msg fail.");
        return;
    }

    /* 清消息空间 */
    PS_MEM_SET((VOS_UINT8*)pstNotchChannelInd + VOS_MSG_HEAD_LENGTH, 0, ulLenth);

    /* 填充消息 */
    pstNotchChannelInd->stMsgHeader.ulReceiverPid   = ulRecvPid;
    pstNotchChannelInd->stMsgHeader.ulMsgName       = ID_MTC_RRC_NOTCH_CHANNEL_IND;
    pstNotchChannelInd->enStartFlag                 = enEnableFlag;

    /* 发送消息 */
    if (VOS_OK != PS_SEND_MSG(UEPS_PID_MTC, pstNotchChannelInd))
    {
        MTC_ERROR1_LOG("MTC_SndRrcNotchChannelInd: Snd Msg err. Rec Pid ", ulRecvPid);
        return;
    }

    return;
}






PS_BOOL_ENUM_UINT8 MTC_CheckGsmWithB31(
    RRC_MTC_MS_BAND_INFO_STRU          *pulBandInfo
)
{
    /* 通过读取Bit位获取当前Band状态 */
    if (0 != (pulBandInfo->aulBandInfo[0] & (VOS_UINT32)((VOS_UINT32)1 << MTC_BAND_B31_BIT)))
    {
        return PS_TRUE;
    }
    else
    {
        return PS_FALSE;
    }
}
PS_BOOL_ENUM_UINT8 MTC_CheckGsmWithB32(
    RRC_MTC_MS_BAND_INFO_STRU          *pulBandInfo
)
{
    /* 通过读取Bit位获取当前Band状态 */
    if (0 != (pulBandInfo->aulBandInfo[0] & (VOS_UINT32)((VOS_UINT32)1 << MTC_BAND_B32_BIT)))
    {
        return PS_TRUE;
    }
    else
    {
        return PS_FALSE;
    }
}
VOS_VOID MTC_SndRrcBandCfgInd(
    VOS_UINT32                          ulRecvPid,
    PS_BOOL_ENUM_UINT8                  enEnableFlag
)
{
    VOS_UINT16                          ulLenth;
    MTC_RRC_BAND_CFG_IND_STRU          *pstBandCfgInd = VOS_NULL_PTR;

    ulLenth = sizeof(MTC_RRC_BAND_CFG_IND_STRU) - VOS_MSG_HEAD_LENGTH;

    /* 分配消息空间 */
    pstBandCfgInd  = (MTC_RRC_BAND_CFG_IND_STRU *)PS_ALLOC_MSG(UEPS_PID_MTC, ulLenth);

    if (VOS_NULL_PTR == pstBandCfgInd)
    {
        MTC_ERROR_LOG("MTC_SndRrcBandCfgInd: alloc msg fail.");
        return;
    }

    /* 清消息空间 */
    PS_MEM_SET((VOS_UINT8*)pstBandCfgInd + VOS_MSG_HEAD_LENGTH, 0, ulLenth);

    /* 填充消息 */
    pstBandCfgInd->stMsgHeader.ulReceiverPid   = ulRecvPid;
    pstBandCfgInd->stMsgHeader.ulMsgName       = ID_MTC_RRC_BAND_CFG_IND;
    pstBandCfgInd->enNarrowDcsFlag             = enEnableFlag;

    /* 发送消息 */
    if (VOS_OK != PS_SEND_MSG(UEPS_PID_MTC, pstBandCfgInd))
    {
        MTC_ERROR1_LOG("MTC_SndRrcBandCfgInd: Snd Msg err. Rec Pid ", ulRecvPid);
        return;
    }

    return;
}


VOS_VOID MTC_ChangeNarrowBandDcsState(
    PS_BOOL_ENUM_UINT8                  enEnableFlag
)
{
    /* 如果与当前状态一致则直接返回 */
    if (MTC_GetNarrowDcsEnalbeFlag() == enEnableFlag)
    {
        return;
    }

    /* 只有在该Modem开机时才下发裁决指示 */
    if (MTC_MODEM_POWER_ON == MTC_GetModemPowerState(MODEM_ID_1))
    {
        /* 判断当前接入技术是否支持G模*/
        if (VOS_TRUE == MTC_IsSupportRatType(MODEM_ID_1, MTC_RATMODE_GSM))
        {
            /* 给副卡GAS下发Narrow Band dcs 裁决 */
            MTC_SndRrcBandCfgInd(I1_UEPS_PID_GAS, enEnableFlag);
        }
    }

    /* 记录Notch标志 */
    MTC_SetNarrowDcsEnalbeFlag(enEnableFlag);

    return;
}
VOS_VOID MTC_ProcNarrowDcsStrategy(VOS_VOID)
{
    MTC_MODEM_POWER_STATE_ENUM_UINT8        enGModemPowerState;
#if 0
    MTC_INTRUSION_GSM_BAND_STATE_ENUM_UINT8 enGsmBandState;
    PS_BOOL_ENUM_UINT8                      enWithB31Flag;
    PS_BOOL_ENUM_UINT8                      enWithB32Flag;
#endif
    enGModemPowerState  = MTC_GetModemPowerState(MODEM_ID_1);

    /* G modem未开机时 */
    if (MTC_MODEM_POWER_ON != enGModemPowerState)
    {
        /* 去使能Narrow band dcs */
        MTC_ChangeNarrowBandDcsState(PS_FALSE);
        return;
    }
    else
    {
        MTC_ChangeNarrowBandDcsState(PS_TRUE);
        return;
    }
#if 0
    /* 获取G modem的频段状态 */
    enGsmBandState      = MTC_GetModem1GsmBandState();

    /* 不存在任何频段 */
    if (MTC_INTRUSION_GSM_BAND_NONE == enGsmBandState)
    {
        /* 去使能Narrow band dcs */
        MTC_ChangeNarrowBandDcsState(PS_FALSE);
        return;
    }

    /* 1. 仅存在频段B31，需要Narrow Band Dcs
       2. 存在频段B31以及其他频段，需要Narrow Band Dcs
     */
    enWithB31Flag = MTC_CheckGsmWithB31(MTC_GetModemCurBandInfo(MODEM_ID_1));
    enWithB32Flag = MTC_CheckGsmWithB32(MTC_GetModemCurBandInfo(MODEM_ID_1));

    /* 存在B31且不存在B32的场景下，需要Narrow Band Dcs */
    if ((PS_TRUE  == enWithB31Flag)
     && (PS_FALSE == enWithB32Flag))
    {
        /* 使能Narrow band dcs */
        MTC_ChangeNarrowBandDcsState(PS_TRUE);
    }
    else
    {
        /* 去使能Narrow band dcs */
        MTC_ChangeNarrowBandDcsState(PS_FALSE);
    }
    return;
#endif
}
VOS_VOID MTC_SndI1GasRrcTdsLteRfCtrlInd(PS_BOOL_ENUM_UINT8 enUplinkCloseFlag)
{
    MTC_RRC_TDS_LTE_RF_CONTROL_IND_STRU    *pstMsg = VOS_NULL_PTR;

    /* 申请消息 */
    pstMsg = (MTC_RRC_TDS_LTE_RF_CONTROL_IND_STRU *)MTC_ALLOC_MSG_WITH_HDR(
                            sizeof(MTC_RRC_TDS_LTE_RF_CONTROL_IND_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        MTC_ERROR_LOG("MTC_SndI1GasRrcTdsLteRfCtrlInd: Alloc msg fail.");
        return;
    }

    /* 清空消息内容 */
    PS_MEM_SET(MTC_GET_MSG_ENTITY(pstMsg), 0x00, MTC_GET_MSG_LENGTH(pstMsg));

    /* 填写消息头 */
    MTC_CFG_I1_GAS_MSG_HDR(pstMsg, ID_MTC_RRC_TDS_LTE_RF_CONTROL_IND);

    /* 填写消息内容 */
    pstMsg->enUplinkCloseFlag = enUplinkCloseFlag;

    /* 发送消息 */
    MTC_SEND_MSG(pstMsg);
    return;
}



PS_BOOL_ENUM_UINT8 MTC_ProcB39B32IntrusionStrategy(VOS_VOID)
{
    RRC_MTC_MS_BAND_INFO_STRU          *pstSerCellBandInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulB39Ret;
    MTC_MODEM_POWER_STATE_ENUM_UINT8    enModem1PowerState;
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8   enModem0PsState;
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8   enModem1PsState;
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8   enModem1CsState;
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8   enModem0CsState;

    /* 初始化 */
    enModem0PsState     = TAF_MTC_SRV_CONN_STATE_BUTT;
    enModem0CsState     = TAF_MTC_SRV_CONN_STATE_BUTT;
    enModem1PsState     = TAF_MTC_SRV_CONN_STATE_BUTT;
    enModem1CsState     = TAF_MTC_SRV_CONN_STATE_BUTT;
    ulB39Ret            = PS_FALSE;
    enModem1PowerState  = MTC_MODEM_POWER_STATE_BUTT;

    /* 判断modem1是否开机 */
    enModem1PowerState =  MTC_GetModemPowerState(MODEM_ID_1);
    if (MTC_MODEM_POWER_OFF == enModem1PowerState)
    {
        /* 打孔方案不生效 */
        return PS_FALSE;
    }

    /* 判断主卡是否存在B39 */
    pstSerCellBandInfo = MTC_GetModemSerCellBandInfo(MODEM_ID_0);
    ulB39Ret = MTC_BAND_CHECK64(*((VOS_UINT64 *)&(pstSerCellBandInfo->aulBandInfo[0])), MTC_BAND_MASK_B39);
    if (VOS_FALSE == ulB39Ret)
    {
        /* 打孔方案不生效 */
        return PS_FALSE;
    }

    /* 判断主副卡的业务状态 */
    enModem0PsState = MTC_GetRcmPsConnStateInfo(MODEM_ID_0);
    enModem0CsState = MTC_GetCsConnStateInfo(MODEM_ID_0);
    enModem1PsState = MTC_GetRcmPsConnStateInfo(MODEM_ID_1);
    enModem1CsState = MTC_GetCsConnStateInfo(MODEM_ID_1);

    /* 主卡存在CS,则打孔方案不生效 */
    if (TAF_MTC_SRV_EXIST == enModem0CsState)
    {
        return PS_FALSE;
    }

    /* 副卡CS, 则打孔方案生效 */
    if (TAF_MTC_SRV_EXIST == enModem1CsState)
    {
        return PS_TRUE;
    }

    /* 主卡PS,副卡IDLE, 则打孔方案生效 */
    if ( (TAF_MTC_SRV_EXIST == enModem0PsState)
      && ( (TAF_MTC_SRV_EXIST != enModem1CsState)
         && (TAF_MTC_SRV_EXIST != enModem1PsState) ) )
    {
        return PS_TRUE;
    }

    /* 其它情况打孔方案不生效 */
    return PS_FALSE;
}
VOS_VOID MTC_ProcB39B32Intrusion(VOS_VOID)
{
    PS_BOOL_ENUM_UINT8                  ulStrategyRet;
    PS_BOOL_ENUM_UINT8                  enEnableFlag;

    /* 判断裁决结果与之前是否一致，如果一样，不再发送消息 */
    enEnableFlag    = MTC_GetTlRfCtrlEnalbeFlag();
    ulStrategyRet   = MTC_ProcB39B32IntrusionStrategy();
    if (enEnableFlag == ulStrategyRet)
    {
        return;
    }

    MTC_SetTlRfCtrlEnalbeFlag(ulStrategyRet);
    MTC_SndI1GasRrcTdsLteRfCtrlInd(ulStrategyRet);

    return;
}
VOS_VOID MTC_SndRrcRseCfgInd(
    VOS_UINT32                          ulRcvPid,
    PS_BOOL_ENUM_UINT8                  enRseOnFlag
)
{
    MTC_RRC_RSE_CFG_IND_STRU           *pstRseCfg = VOS_NULL_PTR;
    VOS_UINT32                          ulLength;

    ulLength = sizeof(MTC_RRC_RSE_CFG_IND_STRU) - VOS_MSG_HEAD_LENGTH;

    /* 分配消息空间 */
    pstRseCfg = (MTC_RRC_RSE_CFG_IND_STRU *)PS_ALLOC_MSG(UEPS_PID_MTC, ulLength);
    if (VOS_NULL_PTR == pstRseCfg)
    {
        MTC_ERROR_LOG("MTC_SndRrcRseCfgInd: alloc mem fail.");
        return;
    }

    /* 填写消息内容 */
    pstRseCfg->stMsgHeader.ulReceiverPid    = ulRcvPid;
    pstRseCfg->stMsgHeader.ulMsgName        = ID_MTC_RRC_RSE_CFG_IND;
    pstRseCfg->enRseOnFlag                  = enRseOnFlag;
    pstRseCfg->aucRsv[0]                    = 0;
    pstRseCfg->aucRsv[1]                    = 0;
    pstRseCfg->aucRsv[2]                    = 0;

    /* 发送消息 */
    if (VOS_OK != PS_SEND_MSG(UEPS_PID_MTC, pstRseCfg))
    {
        MTC_ERROR_LOG("MTC_SndRrcRseCfgInd: Snd Msg err.");
        return;
    }

    /* 将上报结果保存到上下文中 */
    MTC_SetRseCfgValue(enRseOnFlag);

    return;
}


VOS_VOID MTC_ProcRseStrategy(VOS_VOID)
{
    PS_BOOL_ENUM_UINT8                  enRseOnFlag;
    MTC_MODEM_POWER_STATE_ENUM_UINT8    enModem0PowerState;
    MTC_MODEM_POWER_STATE_ENUM_UINT8    enModem1PowerState;

    /* 判断当前是否支持RSE功能 */
    if (MTC_CFG_DISABLE == MTC_GetRseSupportCfg())
    {
        return;
    }

    /* 判断是否是测试卡 */
    if (VOS_FALSE == USIMM_IsTestCard_Instance(MODEM_ID_1))
    {
        return;
    }

    /* 初始化 */
    enRseOnFlag        = PS_FALSE;
    enModem0PowerState = MTC_GetModemPowerState(MODEM_ID_0);
    enModem1PowerState = MTC_GetModemPowerState(MODEM_ID_1);

    /* 若modem0关机 modem1开机，则需要RSE ON */
    if ((MTC_MODEM_POWER_OFF == enModem0PowerState )
      &&(MTC_MODEM_POWER_ON  == enModem1PowerState))
    {
        enRseOnFlag = PS_TRUE;
    }

    /* 若与之前的值相同，则不再下发消息 */
    if (enRseOnFlag == MTC_GetRseCfgValue())
    {
        return ;
    }

    /* 给modem1发送RSE配置消息 */
    MTC_SndRrcRseCfgInd(I1_UEPS_PID_GAS, enRseOnFlag);

}

#if (FEATURE_ON == FEATURE_LTE)

VOS_VOID MTC_SndLrrcModem1InfoInd(VOS_VOID)
{
    MTC_RRC_MODEM1_INFO_IND_STRU       *pstModem1InfoInd    = VOS_NULL_PTR;
    MTC_MODEM_INFO_STRU                *pstModem1Info       = VOS_NULL_PTR;
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8   enCsConnState;

    pstModem1Info       = MTC_GetModemCtxAddr(MODEM_ID_1);
    enCsConnState       = MTC_GetCsConnStateInfo(MODEM_ID_1);

    /* 仅在G900&LTE B7干扰控制功能打开时才发送此消息 */
    if (MTC_CFG_ENABLE != MTC_GetG900B7Cfg())
    {
        /* 功能未开启则不发送 */
        return;
    }

    /* Modem0开机才给LRRC发送Modem1的信息 */
    if (MTC_MODEM_POWER_ON != MTC_GetModemPowerState(MODEM_ID_0))
    {
        /* Modem0未开机则不发送 */
        return;
    }

    /* 分配消息空间 */
    pstModem1InfoInd    = (MTC_RRC_MODEM1_INFO_IND_STRU *)MTC_ALLOC_MSG_WITH_HDR(
                                sizeof(MTC_RRC_MODEM1_INFO_IND_STRU));

    if (VOS_NULL_PTR == pstModem1InfoInd)
    {
        MTC_ERROR_LOG("MTC_SndLrrcModem1InfoInd: Alloc msg fail!");
        return;
    }

    /* 清消息空间 */
    PS_MEM_SET(MTC_GET_MSG_ENTITY(pstModem1InfoInd), 0x00, MTC_GET_MSG_LENGTH(pstModem1InfoInd));

    /* 填充消息 */
    MTC_CFG_LRRC_MSG_HDR(pstModem1InfoInd, ID_MTC_RRC_MODEM1_INFO_IND);

    if (TAF_MTC_SRV_EXIST == enCsConnState)
    {
        pstModem1InfoInd->enCellState = RRC_MTC_CELL_STATE_CONNECT;
    }
    else
    {
        pstModem1InfoInd->enCellState = RRC_MTC_CELL_STATE_IDLE;
    }

    pstModem1InfoInd->enCountry = MTC_GetCountryFromMcc(pstModem1Info->stNetworkInfo.stPlmnId.ulMcc);

    /* 如果Modem1 GAS指示的业务小区频段为空，则使用服务小区频段，否则使用业务小区频段 */
    if ( (0 == pstModem1Info->stServiceBandInfo.aulBandInfo[0])
      && (0 == pstModem1Info->stServiceBandInfo.aulBandInfo[1]) )
    {
        pstModem1InfoInd->stSerCellBandInfo = pstModem1Info->stSerCellBandInfo;
    }
    else
    {
        pstModem1InfoInd->stSerCellBandInfo = pstModem1Info->stServiceBandInfo;
    }

    /* 发送消息 */
    MTC_SEND_MSG(pstModem1InfoInd);

    return;
}
#endif





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif



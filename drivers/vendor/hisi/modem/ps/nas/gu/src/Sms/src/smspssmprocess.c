
#include "smsinclude.h"
#include "LLInterface.h"
#include "NasGmmInterface.h"
#if (FEATURE_ON == FEATURE_LTE)
#include "LmmSmsInterface.h"
#endif

#ifdef  __cplusplus
  #if  __cplusplus
  extern "C"{
  #endif
#endif


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_SMS_PSSMPROCESS_C
extern VOS_UINT8  Smt_StartMemNotification( VOS_VOID );

/*lint -save -e958 */


VOS_UINT32 NAS_SMS_ResendPsRpData(VOS_UINT8 ucRetransFlg)
{
    VOS_UINT8                           ucRdPos;
    VOS_UINT8                           ucTpType;

    if ((VOS_NULL_PTR == g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg)
     || (0 == g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen))
    {
        return VOS_FALSE;
    }

#if (FEATURE_ON == FEATURE_LTE)
    if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
    {

        if (SMS_TRUE != g_SmrEnt.SmrMo.ucMemAvailFlg)
        {
            ucRdPos = (VOS_UINT8)(SMS_TPDU_MTI_LTE_PS + *((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg + SMS_TPDU_SCLEN_LTE_PS));
            ucTpType = (*((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg + ucRdPos)) & NAS_SMS_TP_MTI_MASK;
            if ((NAS_SMS_TP_MTI_SUBMIT == ucTpType)
             && (VOS_TRUE == ucRetransFlg))
            {
                *((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg + ucRdPos) |= SMT_TP_RD_RETRANS;
            }
        }

        NAS_SMS_ReSendLmmCpDataReq(VOS_TRUE);
    }
    else
#endif
    {
        if (SMS_TRUE != g_SmrEnt.SmrMo.ucMemAvailFlg)
        {
            ucRdPos = SMS_TPDU_MTI_PS + *((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg + SMS_TPDU_SCLEN_PS);
            ucTpType = (*((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg + ucRdPos)) & NAS_SMS_TP_MTI_MASK;

            if ((NAS_SMS_TP_MTI_SUBMIT == ucTpType)
             && (VOS_TRUE == ucRetransFlg))
            {
                *((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg + ucRdPos) |= SMT_TP_RD_RETRANS;
            }
        }

        NAS_SMS_ReSendGmmCpDataReq(VOS_TRUE);
    }

    return VOS_TRUE;
}
VOS_UINT32 NAS_SMS_ResendPsRpReport(VOS_VOID)
{
    if ((VOS_NULL_PTR == g_SmcPsEnt.SmcMt.HoldSndMsg.pMsg)
     || (0 == g_SmcPsEnt.SmcMt.HoldSndMsg.ulMsgLen))
    {
        return VOS_FALSE;
    }

#if (FEATURE_ON == FEATURE_LTE)
    if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
    {
        NAS_SMS_ReSendLmmCpDataReq(VOS_FALSE);
    }
    else
#endif
    {
        NAS_SMS_ReSendGmmCpDataReq(VOS_FALSE);
    }

    return VOS_TRUE;
}


/*******************************************************************************
  Module:      SMC_ReportM2NOtaMsg
  Function:    在trace中勾取SMS的空口消息
  Input:       NAS_MSG_STRU*   pNasMsg     SMS空口消息
  NOTE:
  Return:      VOS_VOID
  History:
      1. 鲁琳    2009.07.11   新规作成
*******************************************************************************/

VOS_VOID SMC_ReportM2NOtaMsg(NAS_MSG_STRU *pNasMsg)
{
    NAS_OTA_MSG_ID_ENUM_UINT16 usNasOtaMsyId = NAS_OTA_MSG_ID_BUTT;

    PS_LOG1(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_INFO, "SMC_ReportM2NOtaMsg:NORMAL: msg type is ", pNasMsg->aucNasMsg[1]);

    switch(pNasMsg->aucNasMsg[1])
    {
        case SMC_DATA_TYPE_CP_DATA:
            usNasOtaMsyId = NAS_OTA_MSG_SMS_DATA_TYPE_CP_DATA_UP;
            break;

        case SMC_DATA_TYPE_CP_ACK:
            usNasOtaMsyId = NAS_OTA_MSG_SMS_DATA_TYPE_CP_ACK_UP;
            break;

        case SMC_DATA_TYPE_CP_ERR:
            usNasOtaMsyId = NAS_OTA_MSG_SMS_DATA_TYPE_CP_ERROR_UP;
            break;

        default:
            PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_WARNING, "SMC_ReportM2NOtaMsg:WARNING: invalid msg id");
            /*不属于SMS的空口消息，不在trace中显示*/
            return;
    }

    NAS_SendAirMsgToOM(WUEPS_PID_SMS, usNasOtaMsyId, NAS_OTA_DIRECTION_UP, pNasMsg->ulNasMsgSize + 4, (VOS_UINT8*)pNasMsg);
}
VOS_VOID SMC_RcvPsMtData(
                     VOS_UINT8* pucCpdu,
                     VOS_UINT32  ulCpduLen
                     )
{
    VOS_UINT8                           ucResult;
    VOS_UINT32                          ulRet ;
#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT8                           ucTi;
#endif

    ucResult = SMC_ComNwMsgChk( pucCpdu, ulCpduLen );                           /* 消息检查                                 */

#if (FEATURE_ON == FEATURE_LTE)
    ucTi     = (pucCpdu[0] & NAS_SMS_MSG_SND_FROM_SIDE_THAT_ORIGINATES_THE_TI) >> 4;
#endif

    if(SMC_MSG_VALID != ucResult)
    {                                                                           /* 结果错误                                 */
        if(SMC_CP_ERR_MSG_INVALID != ucResult)
        {
#if (FEATURE_ON == FEATURE_LTE)
            if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
            {
                /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
                NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ERR, ucResult, ucTi);
            }
            else
#endif
            {
                SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ERR,
                                   ucResult,
                                   (pucCpdu[0] & 0xf0) >> 4);                      /* 向网侧回错误指示                         */
            }
        }
        PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_WARNING, "SMC_RcvPsMtData:WARNING:SMC_MSG_VALID != ucResult ");
        return;
    }

    switch(pucCpdu[1])
    {                                                                           /* 消息类型                                 */
    case SMC_DATA_TYPE_CP_DATA:
        if( ( SMC_MT_IDLE == g_SmcPsEnt.SmcMt.ucState )
            || ((SMC_MT_WAIT_FOR_CP_ACK == g_SmcPsEnt.SmcMt.ucState )
             &&(g_SmcPsEnt.SmcMt.ucTi != (pucCpdu[0] >> 4))) )
        {                                                                       /* 实体状态是空闲或在等待网侧确认           */
            if( 0 != g_SmcPsEnt.SmcMt.HoldSndMsg.ulMsgLen )
            {                                                                   /* 有缓存的消息                             */
                SMC_ComPsWaitAckSta(SMS_TRUE);                                  /* 调用公共处理                             */
            }
            PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_RcvPsMtData:NORMAL: SMS state = SMC_MT_WAIT_FOR_RP_ACK");
            g_SmcPsEnt.SmcMt.ucState = SMC_MT_WAIT_FOR_RP_ACK;                  /* 状态迁移                                 */

            g_SmcPsEnt.SmcMt.ucTi = pucCpdu[0] >> 4;                            /* 存储ti                                   */

            if ( ulCpduLen > SMC_MSG_HDR_LEN + 2  )
            {
                g_SmcPsEnt.SmcMt.ucMr = pucCpdu[SMC_MSG_HDR_LEN + 2];               /* 存储mr                                   */
            }

#if (FEATURE_ON == FEATURE_LTE)
            if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
            {
                /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
                NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ACK, 0XFF, g_SmcPsEnt.SmcMt.ucTi);
            }
            else
#endif
            {
                SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ACK,
                                   0XFF ,
                                   g_SmcPsEnt.SmcMt.ucTi );                         /* 向网侧回确认                             */
            }
            ulRet = SMR_SmcApiEstInd( &(pucCpdu[3]),
                              (ulCpduLen - SMC_MSG_HDR_LEN) - 1,
                              SMS_CN_PS );                /* 向高层提交数据                           */
            if (VOS_OK != ulRet)
            {
                if (SMR_ERR_CAUSE_MSG_INVALID_INFO_ERROR == ulRet)
                {
                    g_SmcPsEnt.SmcMt.ucState = SMC_MT_IDLE;;
                }
                else
                {
                    g_SmcPsEnt.SmcMt.ucState = SMC_MT_WAIT_FOR_CP_ACK;
                }
            }
        }
        else if((SMC_MT_WAIT_FOR_CP_ACK == g_SmcPsEnt.SmcMt.ucState )
             &&(g_SmcPsEnt.SmcMt.ucTi == (pucCpdu[0] >> 4)))
        {
#if (FEATURE_ON == FEATURE_LTE)
            if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
            {
                /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
                NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ERR, SMS_CP_ERR_INVALID_TI, ucTi);
            }
            else
#endif
            {
                SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ERR,
                                   SMS_CP_ERR_INVALID_TI,
                                   (pucCpdu[0] & 0xf0) >> 4);                       /* 向网侧指示错误                           */
            }
        }

        else if( SMC_MT_WAIT_FOR_RP_ACK == g_SmcPsEnt.SmcMt.ucState )
        {                                                                       /* 实体在等待网侧确认                       */
#if (FEATURE_ON == FEATURE_LTE)
            if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
            {
                /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
                NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ERR, SMS_CP_ERR_MSG_INCOMP_WITH_PROT_STA, ucTi);
            }
            else
#endif
            {
                SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ERR,
                                   SMS_CP_ERR_MSG_INCOMP_WITH_PROT_STA,
                                   (pucCpdu[0] & 0xf0) >> 4);                       /* 向网侧指示错误                           */
            }
        }
        else
        {

        }
        break;
    case SMC_DATA_TYPE_CP_ACK:
        if( SMC_MT_WAIT_FOR_CP_ACK == g_SmcPsEnt.SmcMt.ucState )
        {                                                                       /* 与实体的状态兼容                         */
            SMC_ComPsWaitAckSta(SMS_TRUE);                                      /* 调用公共处理                             */
            PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_RcvPsMtData:NORMAL: SMS state = SMC_MT_IDLE ");
            g_SmcPsEnt.SmcMt.ucState = SMC_MT_IDLE;                             /* 状态迁移                                 */
        }
        else
        {
#if (FEATURE_ON == FEATURE_LTE)
            if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
            {
                /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
                NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ERR,
                                      SMS_CP_ERR_MSG_INCOMP_WITH_PROT_STA, ucTi);
            }
            else
#endif
            {
                SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ERR,
                                   SMS_CP_ERR_MSG_INCOMP_WITH_PROT_STA,
                                   (pucCpdu[0] & 0xf0) >> 4);                       /* 向网侧指示错误                           */
            }
        }
        break;
    case SMC_DATA_TYPE_CP_ERR:
        if( SMC_MT_IDLE != g_SmcPsEnt.SmcMt.ucState )
        {                                                                       /* 与实体的状态兼容                         */
            SMR_SmcApiErrorInd( (SMR_SMT_ERROR_CP_ERROR_BEGIN | pucCpdu[SMC_MSG_HDR_LEN]), SMS_TRUE);                                     /* 向上层报错                               */

            if( 0 != g_SmcPsEnt.SmcMt.HoldSndMsg.ulMsgLen )
            {                                                                   /* 调用公共处理                             */
                SMC_ComPsWaitAckSta(SMS_TRUE);                                  /* 调用公共处理                             */
            }
            PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_RcvPsMtData:NORMAL: SMS state = SMC_MT_IDLE ");
            g_SmcPsEnt.SmcMt.ucState = SMC_MT_IDLE;                             /* 进入空闲状态                             */
        }
        else
        {

        }
        break;
    default:

#if (FEATURE_ON == FEATURE_LTE)
        if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
        {
            /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
            NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ERR,
                                  SMS_CP_ERR_MSG_TYPE_NOT_IMPLEMENT, ucTi);
        }
        else
#endif
        {
            SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ERR,
                               SMS_CP_ERR_MSG_TYPE_NOT_IMPLEMENT,
                               (pucCpdu[0] & 0xf0) >> 4);                           /* 向网侧指示错误                           */
        }
        break;
    }
    return;
}
VOS_VOID NAS_SMC_BufferPsCpAck(
    VOS_BOOL                            *pbBufferFlag
)
{
    VOS_INT8                            cVersion;
    VOS_UINT8                           ucAccessTech;

    /*PS域短信联发功能关闭: 直接返回不需要缓存CP-ACK消息;*/
    if (NAS_SMS_PS_CONCATENATE_DISABLE == SMS_GetSmsPsConcatenateFlag())
    {
        *pbBufferFlag = VOS_FALSE;
        return;
    }

    /*PS域短信联发功能开启: 根据当前支持的协议版本判断是否需要缓存CP-ACK消息;
        1。UE设置的协议版本为R4以下版本，则不需要缓存CP-ACK消息;
        2。否则，UE设置的协议版本为R4及以上版本:
            UE支持R6及以上版本
                3G接入技术, 需要缓存CP-ACK消息;
                2G接入技术
                    UE设置的协议版本为R6及以上版本, 需要缓存CP-ACK消息;
                    否则，不需要缓存CP-ACK消息;
            UE不支持R6及以上版本
                3G接入技术, 需要缓存CP-ACK消息;
                2G接入技术, 不需要缓存CP-ACK消息;
    */
    cVersion = NAS_Common_Get_Supported_3GPP_Version(SMS_MM_COM_SRVDOMAIN_PS);
    #if (PS_UE_REL_VER >= PS_PTL_VER_R6)
    if (cVersion >= PS_PTL_VER_R6)
    {
        *pbBufferFlag = VOS_TRUE;
        return;
    }
    else if (cVersion >= PS_PTL_VER_R4)
    #else
    if (cVersion >= PS_PTL_VER_R4)
    #endif
    {
        ucAccessTech = GMM_GetCurNetwork();
        if (NAS_GMM_NET_RAT_TYPE_GSM == ucAccessTech)
        {
            *pbBufferFlag = VOS_FALSE;
            return;
        }
        else
        {
            *pbBufferFlag = VOS_TRUE;
            return;
        }
    }
    else
    {
        *pbBufferFlag = VOS_FALSE;
        return;
    }

}


VOS_VOID SMC_RcvPsMoCpData(
                     VOS_UINT8* pucCpdu,
                     VOS_UINT32  ulCpduLen
                    )
{
    VOS_BOOL                            bBufferFlag;
    VOS_UINT32                          ulRet;
    VOS_UINT8                           ucOrigStatus;

    if ( (g_SmcPsEnt.SmcMo.ucState != SMC_MO_WAIT_FOR_CP_DATA)
      && (g_SmcPsEnt.SmcMo.ucState != SMC_MO_WAIT_FOR_CP_ACK) )
    {
        return;
    }

    ucOrigStatus = g_SmcPsEnt.SmcMo.ucState;
    NAS_SMC_BufferPsCpAck(&bBufferFlag);
    if (VOS_TRUE == bBufferFlag)
    {
        PS_LOG(WUEPS_PID_SMS,
               VOS_NULL,
               PS_PRINT_NORMAL,
               "SMC_RcvPsMoCpData: SMS state = SMC_MO_WAIT_TO_SND_CP_ACK ");
        g_SmcPsEnt.SmcMo.ucState = SMC_MO_WAIT_TO_SND_CP_ACK;                             /* 状态迁移                                 */
        g_SmcPsEnt.SmcMo.ucCpAckFlg = SMS_TRUE;                                 /* 设置需要发送CP-ACK标志                   */
    }
    else
    {
#if (FEATURE_ON == FEATURE_LTE)
        if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
        {
            /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
            NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ACK, 0XFF, g_SmcPsEnt.SmcMo.ucTi);
        }
        else
#endif
        {
            SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ACK,
                               0XFF,
                               g_SmcPsEnt.SmcMo.ucTi );                             /* 向网侧回确认                             */
        }
        PS_LOG(WUEPS_PID_SMS,
               VOS_NULL,
               PS_PRINT_NORMAL,
               "SMC_RcvPsMoCpData: SMS state = SMC_MO_IDLE ");
        g_SmcPsEnt.SmcMo.ucState = SMC_MO_IDLE;                                 /* 状态迁移                                 */
        SMS_SendMnMoLinkCloseInd();
    }

    ulRet = SMR_SmcApiDataInd( &(pucCpdu[3] ),
                              (ulCpduLen - SMC_MSG_HDR_LEN) -1 );                /* 向高层提交数据                           */
    if(VOS_OK == ulRet)
    {
        /*MO处于SMC_MO_WAIT_FOR_CP_ACK状态说明没有接收到CP-ACK直接接收RP-ACK/ERR*/
        if (SMC_MO_WAIT_FOR_CP_ACK == ucOrigStatus)
        {
            /* 停止timer TC1M */
            SMS_LOG( PS_PRINT_NORMAL, "SMS:SMC_RcvPsMoCpData: TimerStop: TC1M" );
            if (SMS_FALSE == SMS_TimerStop( SMC_TIMER_ID_CS_TC1M_MO ) )
            {
                SMS_LOG( PS_PRINT_WARNING, "SMS:SMC_RcvPsMoCpData: fail to Stop TC1M" );
            }

            if (0 != g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen)
            {                                                                   /* 有存储的发送消息                         */
                SMS_Free(g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg);                     /* 释放存储的消息                           */
                g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen = 0;                       /* 清除消息长度                             */
                g_SmcPsEnt.SmcMo.enHoldSndMsgType    = NAS_SMS_HOLD_MSG_BUTT;
            }

            g_SmcPsEnt.SmcMo.TimerInfo.ucExpireTimes = 0;                        /* 清除timer超时次数                        */
        }
    }
    else
    {
        PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_RcvPsMoCpData:Invalid Data,so change State ");
        if (SMR_ERR_CAUSE_MSG_PROTOCOL_ERROR != ulRet)
        {
            g_SmcPsEnt.SmcMo.ucState = SMC_MO_WAIT_FOR_CP_ACK;            /* 状态迁移                                 */
        }
        else
        {
            g_SmcPsEnt.SmcMo.ucState = ucOrigStatus;            /* 状态迁移                                 */
        }

        g_SmcPsEnt.SmcMo.ucCpAckFlg = SMS_FALSE;
        return;
    }

    return;
}


VOS_VOID SMC_RcvPsMoData(
                     VOS_UINT8* pucCpdu,
                     VOS_UINT32  ulCpduLen
                    )
{
    VOS_UINT8                           ucResult;

#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT8                           ucTi;
#endif

    ucResult = SMC_ComNwMsgChk( pucCpdu, ulCpduLen );                           /* 消息检查                                 */

#if (FEATURE_ON == FEATURE_LTE)
    ucTi     = (pucCpdu[0] & NAS_SMS_MSG_SND_FROM_SIDE_THAT_ORIGINATES_THE_TI) >> 4;
#endif

    if (SMC_MSG_VALID != ucResult)
    {                                                                           /* 结果错误                                 */
        if (SMC_CP_ERR_MSG_INVALID != ucResult)
        {
#if (FEATURE_ON == FEATURE_LTE)
            if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
            {
                /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
                NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ERR, ucResult, ucTi);
            }
            else
#endif
            {
                SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ERR,
                                   ucResult,
                                   (pucCpdu[0] & 0xf0) >> 4);                       /* 向网侧回错误指示                         */
            }
        }
        PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_WARNING, "SMC_RcvPsMtData:WARNING:SMC_MSG_VALID != ucResult ");
        return;
    }

    switch(pucCpdu[1])
    {                                                                           /* 消息类型                                 */
    case SMC_DATA_TYPE_CP_DATA:
        SMC_RcvPsMoCpData(pucCpdu, ulCpduLen);
        break;
    case SMC_DATA_TYPE_CP_ACK:
        if( SMC_MO_WAIT_FOR_CP_ACK == g_SmcPsEnt.SmcMo.ucState )
        {                                                                       /* 状态正确                                 */
            SMC_ComPsWaitAckSta(SMS_FALSE);                                     /* 调用公共处理                             */

            PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_RcvPsMoData:NORMAL: SMS state = SMC_MO_WAIT_FOR_CP_DATA ");
            g_SmcPsEnt.SmcMo.ucState = SMC_MO_WAIT_FOR_CP_DATA;                 /* 状态迁移                                 */
        }
        else
        {                                                                       /* 在等待网侧的数据信息                     */
#if (FEATURE_ON == FEATURE_LTE)
            if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
            {
                /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
                NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ERR,
                                SMS_CP_ERR_MSG_INCOMP_WITH_PROT_STA, ucTi);
            }
            else
#endif
            {
                SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ERR,
                                   SMS_CP_ERR_MSG_INCOMP_WITH_PROT_STA,
                                   (pucCpdu[0] & 0xf0) >> 4);                       /* 向网侧指示错误                           */
            }
        }
        break;
    case SMC_DATA_TYPE_CP_ERR:
        if( g_SmcPsEnt.SmcMo.ucState == SMC_MO_WAIT_FOR_CP_ACK )
        {                                                                       /* 状态是等待CP-ACK                         */
            SMC_ComPsWaitAckSta(SMS_FALSE);                                     /* 调用公共处理                             */

            SMR_SmcApiErrorInd( (SMR_SMT_ERROR_CP_ERROR_BEGIN | pucCpdu[2]), SMS_FALSE);                        /* 向上层报错                               */

            PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_RcvPsMoData:NORMAL: SMS state = SMC_MO_IDLE ");
            g_SmcPsEnt.SmcMo.ucState = SMC_MO_IDLE;                             /* 状态迁移到空闲                           */

        }
        else if( SMC_MO_WAIT_FOR_CP_DATA == g_SmcPsEnt.SmcMo.ucState )
        {                                                                       /* 状态是等待CP-DAT                         */
            if( SMS_CP_ERR_INVALID_TI == pucCpdu[2] )
            {
                SMR_SmcApiErrorInd( (SMR_SMT_ERROR_CP_ERROR_BEGIN | SMS_CP_ERR_PROT_ERR_UNSPEC), SMS_FALSE );
            }
            else
            {
                SMR_SmcApiErrorInd( (SMR_SMT_ERROR_CP_ERROR_BEGIN | pucCpdu[2]), SMS_FALSE );
            }

            PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_RcvPsMoData:NORMAL: SMS state = SMC_MO_IDLE ");
            g_SmcPsEnt.SmcMo.ucState = SMC_MO_IDLE;                             /* 状态迁移到空闲                           */
        }
        else
        {

        }
        break;
    default:

#if (FEATURE_ON == FEATURE_LTE)
        if (NAS_GMM_NET_RAT_TYPE_LTE == GMM_GetCurNetwork())
        {
            /* 当前驻留在L模,构造SMS_LMM_DATA_REQ消息，通过NAS_SMS_SndLmmDataReq发送 */
            NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ERR,
                                  SMS_CP_ERR_MSG_TYPE_NOT_IMPLEMENT, ucTi);
        }
        else
#endif
        {
            SMC_SndGmmDataReq( SMC_DATA_TYPE_CP_ERR,
                               SMS_CP_ERR_MSG_TYPE_NOT_IMPLEMENT,
                               (pucCpdu[0] & 0xf0) >> 4);                           /* 向上层报错                               */
        }
        break;
    }
}


VOS_VOID SMC_RcvGmmEstCnf()
{
    VOS_UINT16 usMsgLen;
    VOS_UINT8 *pucData = SMS_POINTER_NULL;
    VOS_UINT8  aucTempMsg[255];

    if( 0 != g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen )
    {                                                                           /* 有缓存的消息                             */
        if ( SMS_FALSE == SMS_TimerStop( SMC_TIMER_ID_TR1M ) )
        {
            SMS_LOG( PS_PRINT_ERROR,
                     "SMC_RcvGmmEstCnf: SMC_TIMER_ID_TR1M Timer stop fails" );
        }

        if ( SMS_FALSE == SMS_TimerStart( SMC_TIMER_ID_TR1M ) )
        {
            SMS_LOG( PS_PRINT_WARNING, "SMC_RcvGmmEstCnf: TimerStart: TR1M Failed" );
        }

        g_SmcPsEnt.SmcMo.ucTi = ((g_SmcPsEnt.SmcMo.ucTi + 1) % 7) | 0X08;

        g_SmcPsEnt.SmcMo.ucMr = ((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldRcvMsg.pMsg)[2]; /* 记录MR                                   */
        g_SmrEnt.SmrMo.ucMr   = ((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldRcvMsg.pMsg)[2]; /* 记录MR                                   */

        usMsgLen = (VOS_UINT16)(((sizeof(PMMSMS_UNITDATA_REQ_STRU) - 4)
                     + g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen)
                     + SMC_MSG_HDR_LEN);                                         /* 获得消息的长度                           */

        if( g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen < SMC_SND_MSG_MIN_LEN )
        {                                                                       /* 消息的长度较小                           */
           usMsgLen = sizeof(PMMSMS_UNITDATA_REQ_STRU);
        }
        /* pucSnd = (VOS_UINT8 *)Sms_AllocMsg(usMsgLen);                                申请内存                                 */
        SMS_Memset( aucTempMsg,0,usMsgLen );                                        /* 初始化清零                               */

        pucData = ((PMMSMS_UNITDATA_REQ_STRU *)aucTempMsg)->SmsMsg.aucNasMsg;       /* 得到数据首地址                           */

        pucData[0] = (VOS_UINT8)((g_SmcPsEnt.SmcMo.ucTi & 0Xf7) << 4);                       /* 填写TI                                   */
        pucData[0] |= NAS_PD_SMS;                                               /* 填写PD                                   */
        pucData[1] = SMC_DATA_TYPE_CP_DATA;                                     /* 填写消息类型                             */

        SMS_Memcpy( (VOS_UINT8 *)(pucData + 2),
                    (VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldRcvMsg.pMsg,
                    g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen );                     /* 复制数据                                 */

        pucData[2] = (VOS_UINT8)(g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen - 1);         /* 填写消息长度                             */

        ((PMMSMS_UNITDATA_REQ_STRU *)aucTempMsg)->SmsMsg.ulNasMsgSize
              = g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen + SMC_MSG_HDR_LEN;         /* 赋值消息长度                             */

        SMS_Free(g_SmcPsEnt.SmcMo.HoldRcvMsg.pMsg);                             /* 释放消息                                 */

        g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen = 0;                               /* 清除消息长度                             */

        g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg     = (VOS_VOID *)SMS_Malloc(usMsgLen);
        if ( VOS_NULL_PTR == g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg )
        {
            SMS_LOG( PS_PRINT_ERROR,
                     "SMS:SMC_RcvGmmEstCnf: Memory allocation fails" );

            return;
        }
        SMS_Memcpy(g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg, aucTempMsg, usMsgLen);         /* 备份消息                                 */
        g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen = usMsgLen;                        /* 备份消息长度                             */
        g_SmcPsEnt.SmcMo.enHoldSndMsgType    = NAS_SMS_HOLD_MSG_GU_MO_RP_DATA;

        /* 启动定时器TC1M */
        SMS_LOG( PS_PRINT_NORMAL, "SMS:SMC_RcvGmmEstCnf: TimerStart: TC1M" );
        if ( SMS_FALSE == SMS_TimerStart( SMC_TIMER_ID_PS_TC1M_MO ) )
        {
            SMS_LOG( PS_PRINT_WARNING,
                     "SMS:SMC_RcvGmmEstCnf: TimerStart: TC1M Failed" );
            SMS_Free(g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg);
            g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen = 0;                               /* 清除消息长度                             */
            g_SmcPsEnt.SmcMo.enHoldSndMsgType    = NAS_SMS_HOLD_MSG_BUTT;
            return;
        }

        /* g_SmcPsEnt.SmcMo.TimerInfo.ucTimerSta    = SMS_TIMER_STATUS_RUN;         初始化TIMER状态                          */
        g_SmcPsEnt.SmcMo.TimerInfo.ucExpireTimes = 0;

        SMC_SndGmmMsg( aucTempMsg, PMMSMS_UNITDATA_REQ, usMsgLen ) ;                /* 发送消息                                 */

        PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_RcvGmmEstCnf:NORMAL: SMS state = SMC_MO_WAIT_FOR_CP_ACK ");
        g_SmcPsEnt.SmcMo.ucState = SMC_MO_WAIT_FOR_CP_ACK;                      /* 状态迁移                                 */

    }
}

VOS_VOID SMC_ComPsMtErr(SMR_SMT_ERROR_ENUM_UINT32           enErrorCode)
{
    if( SMC_MT_WAIT_FOR_CP_ACK == g_SmcPsEnt.SmcMt.ucState )
    {                                                                           /* MT实体在等待网侧确认                     */
        SMC_ComPsWaitAckSta(SMS_TRUE);                                          /* 调用公共处理                             */
        /* SMR_SmcApiErrorInd(ucCause, SMS_TRUE);                                   上报错误                                 */
    }
    else if( SMC_MT_WAIT_FOR_RP_ACK == g_SmcPsEnt.SmcMt.ucState )
    {                                                                           /* MT实体在等待高层确认                     */
        SMR_SmcApiErrorInd(enErrorCode, SMS_TRUE);                       /* 上报错误                                 */
    }
    else
    {

    }

    PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_ComPsMtErr:NORMAL:SMS state = SMC_MT_IDLE");
    g_SmcPsEnt.SmcMt.ucState = SMC_MT_IDLE;                                     /* 状态迁移到空闲                           */
    return;
}

VOS_VOID SMC_ComPsMoErr(SMR_SMT_ERROR_ENUM_UINT32           enCause)
{
    switch(g_SmcPsEnt.SmcMo.ucState)
    {
    case SMC_MO_WAIT_FOR_CP_ACK:
        SMC_ComPsWaitAckSta(SMS_FALSE);                                         /* 调用公共处理                             */
        SMR_SmcApiErrorInd(enCause, SMS_FALSE);                                 /* 上报错误                                 */
        break;
    case SMC_MO_GMM_CONN_PENDING:
        if (0 != g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen)
        {
            SMS_Free(g_SmcPsEnt.SmcMo.HoldRcvMsg.pMsg);                             /* 释放消息                                 */
            g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen = 0;                               /* 初始化消息长度                           */
        }
        SMR_SmcApiErrorInd(enCause, SMS_FALSE);                                 /* 上报错误                                 */
        break;
    case SMC_MO_WAIT_FOR_CP_DATA:
        SMR_SmcApiErrorInd(enCause, SMS_FALSE);                                 /* 上报错误                                 */
        break;
    case SMC_MO_WAIT_TO_SND_CP_ACK:
        if (SMS_TRUE == g_SmcPsEnt.SmcMo.ucCpAckFlg)
        {
            g_SmcPsEnt.SmcMo.ucCpAckFlg = SMS_FALSE;                             /* 设置需要发送CP-ACK标志                   */
            SMS_SendMnMoLinkCloseInd();
        }
        break;
    default:
        break;
    }

    PS_LOG(WUEPS_PID_SMS, VOS_NULL, PS_PRINT_NORMAL, "SMC_ComPsMoErr:NORMAL:SMS state = SMC_MO_IDLE");
    g_SmcPsEnt.SmcMo.ucState = SMC_MO_IDLE;                                     /* 状态迁移到空闲                           */
}
/*******************************************************************************
  Module:   SMC_ComPsWaitAckSta
  Function: 初始化相应的资源
  Input:    VOS_UINT8   ucMtFlg   MT标识
  Output:   VOS_VOID
  NOTE:
  Return:   VOS_VOID
  History:
      1.  张志勇      2004.03.09   新规作成
      2.
*******************************************************************************/
VOS_VOID SMC_ComPsWaitAckSta(
                         VOS_UINT8   ucMtFlg                                        /* 是mt过程                                 */
                         )
{
    if( SMS_FALSE == ucMtFlg )
    {                                                                           /* 不是mt过程                               */
        if (0 != g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen)
        {
            SMS_Free( g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg );                           /* 释放缓存的消息                           */
            g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen     = 0;                           /* 初始化消息长度                           */
            g_SmcPsEnt.SmcMo.enHoldSndMsgType        = NAS_SMS_HOLD_MSG_BUTT;
        }

        g_SmcPsEnt.SmcMo.TimerInfo.ucExpireTimes = 0;                           /* 清除timer超时次数                        */

        /* 停止timer TC1M */
        SMS_LOG( PS_PRINT_NORMAL, "SMS:SMC_ComPsWaitAckSta: TimerStop: TC1M" );
        if ( SMS_FALSE == SMS_TimerStop( SMC_TIMER_ID_PS_TC1M_MO ) )
        {
            return;
        }

        /* g_SmcPsEnt.SmcMo.TimerInfo.ucTimerSta = SMS_TIMER_STATUS_STOP; */
    }
    else
    {
        if (0 != g_SmcPsEnt.SmcMt.HoldSndMsg.ulMsgLen)
        {
            SMS_Free( g_SmcPsEnt.SmcMt.HoldSndMsg.pMsg );                           /* 释放缓存的消息                           */
            g_SmcPsEnt.SmcMt.HoldSndMsg.ulMsgLen     = 0;                           /* 初始化消息长度                           */
            g_SmcPsEnt.SmcMt.enHoldSndMsgType       = NAS_SMS_HOLD_MSG_BUTT;
        }

        g_SmcPsEnt.SmcMt.TimerInfo.ucExpireTimes = 0;                           /* 清除timer超时次数                        */

        /* 停止timer TC1M */
        SMS_LOG( PS_PRINT_NORMAL, "SMS:SMC_ComPsWaitAckSta: TimerStop: TC1M" );
        if ( SMS_FALSE == SMS_TimerStop( SMC_TIMER_ID_PS_TC1M_MT ) )
        {
            return;
        }

        /* g_SmcPsEnt.SmcMt.TimerInfo.ucTimerSta = SMS_TIMER_STATUS_STOP; */
    }
    return;
}


VOS_VOID SMC_RcvGmmServStatusInd(
                      GMMSMS_SERVICE_STATUS_IND_STRU  *pRcvMsg                  /* 收到的消息首地址                         */
                      )
{
    if (GMM_SMS_NORMAL_SERVICE == pRcvMsg->enServiceSts)
    {                                                                           /* 状态为ATTACH                             */
        g_ucPsServiceStatus = SMS_TRUE;                                         /* 记录此状态                               */
        g_ucCurSendDomain = SMS_SEND_DOMAIN_PS;
    }
    else
    {
        g_ucPsServiceStatus = SMS_FALSE;                                        /* 记录此状态                               */

        SMC_ComPsMtErr(SMR_SMT_ERROR_NO_SERVICE);                                  /* 调用PS域MT实体的处理                     */
        SMC_ComPsMoErr(SMR_SMT_ERROR_NO_SERVICE);

        /* 调用PS域MO实体的处理                     */
    }
    /*通知TAF层*/
    SMS_AttachFlag(SMS_SEND_DOMAIN_PS,g_ucPsServiceStatus);
    return;
}
VOS_VOID SMC_RcvGmmErrInd(
                      PMMSMS_ERROR_IND_STRU  *pRcvMsg                           /* 收到的消息首地址                         */
                      )
{

    SMC_ComPsMtErr(SMR_SMT_ERROR_PS_ERROR_BEGIN | pRcvMsg->ulCause);                                  /* 调用PS域MT实体的处理                     */
    SMC_ComPsMoErr(SMR_SMT_ERROR_PS_ERROR_BEGIN | pRcvMsg->ulCause);                                  /* 调用PS域MO实体的处理                     */

}

/*******************************************************************************
  Module:   SMC_SndGmmDataReq
  Function: 组织并向GMM发送数据
  Input:    VOS_UINT8     ucType   数据类型
            VOS_UINT8     ucCause  错误原因
            VOS_UINT8     ucTi     transaction id
  Output:   VOS_VOID
  NOTE:
  Return:   VOS_VOID
  History:
      1.   张志勇      2004.03.11   新规作成
      2.
*******************************************************************************/
VOS_VOID SMC_SndGmmDataReq(
                        VOS_UINT8     ucType,                                       /* 数据类型                                 */
                        VOS_UINT8     ucCause,                                      /* 错误原因                                 */
                        VOS_UINT8     ucTi                                          /* transaction id                           */
                      )
{
    /* PMMSMS_UNITDATA_REQ_STRU  *pSndMsg = (PMMSMS_UNITDATA_REQ_STRU *) */
    /* Sms_AllocMsg(sizeof(PMMSMS_UNITDATA_REQ_STRU));                  申请空间                                 */
    VOS_UINT8        aucTempMsg[255];

    SMS_Memset( aucTempMsg,0,sizeof(PMMSMS_UNITDATA_REQ_STRU) );                   /* 初始化清零                               */

    ((PMMSMS_UNITDATA_REQ_STRU *)aucTempMsg)->SmsMsg.aucNasMsg[0] = (VOS_UINT8)((ucTi ^ 0x08) << 4);                 /* 填写ti                                   */
    ((PMMSMS_UNITDATA_REQ_STRU *)aucTempMsg)->SmsMsg.aucNasMsg[0] |= NAS_PD_SMS;                                 /* 填写PD                                   */
    ((PMMSMS_UNITDATA_REQ_STRU *)aucTempMsg)->SmsMsg.aucNasMsg[1] = ucType;                                      /* 填写消息类型                             */

    if(SMC_DATA_TYPE_CP_ERR == ucType)
    {                                                                           /* 数据类型是CP_ERROR                       */
        ((PMMSMS_UNITDATA_REQ_STRU *)aucTempMsg)->SmsMsg.ulNasMsgSize = SMC_CP_ERR_LEN;
        ((PMMSMS_UNITDATA_REQ_STRU *)aucTempMsg)->SmsMsg.aucNasMsg[2] = ucCause;
    }
    else
    {                                                                           /* 数据类型是CP_ACK                         */
        ((PMMSMS_UNITDATA_REQ_STRU *)aucTempMsg)->SmsMsg.ulNasMsgSize = SMC_CP_ACK_LEN;
    }
    SMC_SndGmmMsg( aucTempMsg,
                   PMMSMS_UNITDATA_REQ,
                   sizeof(PMMSMS_UNITDATA_REQ_STRU) );                          /* 发送GMM消息                              */

    return;
}

/*lint -e830 -e593*/

VOS_VOID SMC_SndGmmMsg(
                    VOS_UINT8     *pucTempMsg,                                          /* 发送消息的首地址                         */
                    VOS_UINT8     ucType,                                           /* 消息类型                                 */
                    VOS_UINT16    usLen                                             /* 消息长度                                 */
                  )
{
    VOS_UINT16      usCpduLen;                          /* CPDU的长度 */
    VOS_UINT8*      pucCpdu;                            /* 指向CPDU的指针 */
    VOS_UINT32  ulRet;
    LL_NAS_UNITDATA_REQ_MSG*    pSndMsg;
    VOS_UINT8      *pucMsg = SMS_POINTER_NULL;

    pucMsg = (VOS_UINT8* )PS_ALLOC_MSG( WUEPS_PID_SMS,
                            usLen - VOS_MSG_HEAD_LENGTH );
    if ( VOS_NULL_PTR == pucMsg )
    {
        SMS_LOG( PS_PRINT_ERROR,
                 "SMS:SMC_RcvMmEstCnf: Message allocation fails" );
        return;
    }

    PS_MEM_CPY(pucMsg, pucTempMsg, usLen);

    ((MSG_HEADER_STRU * )pucMsg)->ulSenderCpuId   = VOS_LOCAL_CPUID;
    ((MSG_HEADER_STRU * )pucMsg)->ulSenderPid     = WUEPS_PID_SMS;
    ((MSG_HEADER_STRU * )pucMsg)->ulReceiverCpuId = VOS_LOCAL_CPUID;
    ((MSG_HEADER_STRU * )pucMsg)->ulReceiverPid   = WUEPS_PID_GMM;
    ((MSG_HEADER_STRU * )pucMsg)->ulLength        = usLen - VOS_MSG_HEAD_LENGTH;
    ((MSG_HEADER_STRU * )pucMsg)->ulMsgName   = ucType;                         /* 填写消息名字                             */

    if ( NAS_GMM_NET_RAT_TYPE_WCDMA == GMM_GetCurNetwork() )
    {   /* 如果当前网络是3G模式 */

        /*Report event begin*/
        if ( PMMSMS_UNITDATA_REQ == ucType )
        {
            SMC_ReportM2NOtaMsg(&(((PMMSMS_UNITDATA_REQ_STRU *)pucTempMsg)->SmsMsg));
        }
        /*Report event end*/

        ulRet = PS_SEND_MSG( WUEPS_PID_SMS, pucMsg );
        if (VOS_OK != ulRet)
        {
            /* 打印错误信息 */
            SMS_LOG( PS_PRINT_ERROR, "SMS:SMC_SndGmmMsg:Error: Send msg fails" );
        }
    }
    else
    {   /* 如果当前网络是2G模式 */
        /* 如果消息类型是PMMSMS_EST_REQ，那么就模拟从GMM向SMC发送一条消息PMMSMS_EST_CNF */
        if ( PMMSMS_EST_REQ == ucType )
        {
            ((MSG_HEADER_STRU * )pucMsg)->ulSenderPid     = WUEPS_PID_GMM;
            ((MSG_HEADER_STRU * )pucMsg)->ulReceiverPid   = WUEPS_PID_SMS;
            ((MSG_HEADER_STRU * )pucMsg)->ulMsgName       = PMMSMS_EST_CNF;

            ulRet = PS_SEND_MSG( WUEPS_PID_SMS, pucMsg );
            if (VOS_OK != ulRet)
            {
                /* 打印错误信息 */
                SMS_LOG( PS_PRINT_ERROR, "SMS:SMC_SndGmmMsg:Error: Send msg fails" );
            }
        }
        else if ( PMMSMS_UNITDATA_REQ == ucType )
        {
            /* 获得PMMSMS_UNITDATA_REQ_STRU消息中包含的CPDU的长度 */
            usCpduLen = (VOS_UINT16)((PMMSMS_UNITDATA_REQ_STRU*)pucMsg)->SmsMsg.ulNasMsgSize;

            pucCpdu = ((PMMSMS_UNITDATA_REQ_STRU*)pucMsg)->SmsMsg.aucNasMsg;

            /* 申请新的LL_NAS_UNITDATA_REQ_MSG结构的空间 */
            pSndMsg = (LL_NAS_UNITDATA_REQ_MSG *) PS_ALLOC_MSG( WUEPS_PID_SMS,
                                    ((usLen + sizeof(LL_NAS_UNITDATA_REQ_MSG)) -
                                    sizeof(PMMSMS_UNITDATA_REQ_STRU)) -
                                    VOS_MSG_HEAD_LENGTH );

            if ( VOS_NULL_PTR == pSndMsg )
            {
                SMS_LOG( PS_PRINT_ERROR,
                         "SMS:SMC_SndGmmMsg: Message allocation fails" );
                PS_FREE_MSG(WUEPS_PID_SMS, pucMsg);
                return;
            }

            /* 对CPDU的长度进行赋值 */
            pSndMsg->stUnitDataReq.usPduLen = usCpduLen;
            /* 拷贝原来PMM结构中的CPDU到新的LLC的结构中 */
            SMS_Memcpy( pSndMsg->stUnitDataReq.aucPdu, pucCpdu, usCpduLen );

            /* 对结构中的其它Field进行赋值 */
            pSndMsg->ulSenderCpuId   = VOS_LOCAL_CPUID;
            pSndMsg->ulSenderPid     = WUEPS_PID_SMS;
            pSndMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
            pSndMsg->ulReceiverPid   = UEPS_PID_LL;

            pSndMsg->usMsgType       = ID_LL_UNITDATA_REQ;
            pSndMsg->usTransId       = LL_SAPI_SMS;

            pSndMsg->stUnitDataReq.ulTlli     = GMM_GetTlli();          /* 获得Tlli值 */
            pSndMsg->stUnitDataReq.ucRadioPri = GMM_GetRaPrioForSms();         /* 最高无线优先级 */
            pSndMsg->stUnitDataReq.ucCiphInd  = Gmm_GetCurrCipherInfo();                      /* 进行加密 */

            PS_FREE_MSG(WUEPS_PID_SMS, pucMsg);

            SMC_ReportM2NOtaMsg(&(((PMMSMS_UNITDATA_REQ_STRU *)pucTempMsg)->SmsMsg));


            ulRet = PS_SEND_MSG( UEPS_PID_LL, pSndMsg );
            if ( VOS_OK != ulRet )
            {
                SMS_LOG( PS_PRINT_ERROR,
                         "SMS:SMC_SndGmmMsg: Message send fails" );
                return;
            }
        }
        else if (PMMSMS_REL_REQ == ucType)
        {
            ((MSG_HEADER_STRU * )pucMsg)->ulSenderPid     = WUEPS_PID_GMM;
            ((MSG_HEADER_STRU * )pucMsg)->ulReceiverPid   = WUEPS_PID_SMS;
            ((MSG_HEADER_STRU * )pucMsg)->ulMsgName       = PMMSMS_REL_REQ;

            ulRet = PS_SEND_MSG( WUEPS_PID_SMS, pucMsg );
            if (VOS_OK != ulRet)
            {
                /* 打印错误信息 */
                SMS_LOG( PS_PRINT_ERROR, "SMS:SMC_SndGmmMsg:Error: Send msg fails" );
            }
        }
        else
        {

        }
    }
    return;
}
/*lint +e830 +e593*/


#if (FEATURE_ON == FEATURE_LTE)
VOS_VOID NAS_SMS_ProcLmmEstCnf(
    VOS_VOID                           *pRcvMsg
)
{
    VOS_UINT16                          usMsgLen;
    VOS_UINT8                          *pucData;
    VOS_UINT8                           aucMsg[NAS_SMS_MAX_MSG_SIZE];

    /* 有缓存的消息 */
    if( 0 != g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen )
    {
        if ( SMS_FALSE == SMS_TimerStop( SMC_TIMER_ID_TR1M ) )
        {
            SMS_LOG( PS_PRINT_ERROR,
                     "NAS_SMS_ProcLmmEstCnf: SMC_TIMER_ID_TR1M Timer stop fails" );
        }

        if ( SMS_FALSE == SMS_TimerStart( SMC_TIMER_ID_TR1M ) )
        {
            SMS_LOG( PS_PRINT_WARNING, "NAS_SMS_ProcLmmEstCnf: TimerStart: TR1M Failed" );
        }

        g_SmcPsEnt.SmcMo.ucTi = ((g_SmcPsEnt.SmcMo.ucTi + 1) % 7) | NAS_SMS_MSG_SND_TO_SIDE_THAT_ORIGINATES_THE_TI;

        g_SmcPsEnt.SmcMo.ucMr = ((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldRcvMsg.pMsg)[2]; /* 记录MR */
        g_SmrEnt.SmrMo.ucMr   = ((VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldRcvMsg.pMsg)[2]; /* 记录MR */

        /* 获得消息的长度 */
        usMsgLen = (VOS_UINT16)(((sizeof(SMS_LMM_DATA_REQ_STRU) - 4)
                                + g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen)
                                + SMC_MSG_HDR_LEN);

        /* 消息的长度较小 */
        if( g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen < SMC_SND_MSG_MIN_LEN )
        {
           usMsgLen = sizeof(SMS_LMM_DATA_REQ_STRU);
        }

        PS_MEM_SET(aucMsg, 0, usMsgLen);

        pucData = ((SMS_LMM_DATA_REQ_STRU *)aucMsg)->stSmsMsg.aucSmsMsg;        /* 得到数据首地址 */

        pucData[0] = (VOS_UINT8)((g_SmcPsEnt.SmcMo.ucTi & NAS_SMS_TI_MASK) << 4); /* 填写TI */
        pucData[0] |= NAS_PD_SMS;                                               /* 填写PD */
        pucData[1] = SMC_DATA_TYPE_CP_DATA;                                     /* 填写消息类型*/

        PS_MEM_CPY( (VOS_UINT8 *)(pucData + 2),
                    (VOS_UINT8 *)g_SmcPsEnt.SmcMo.HoldRcvMsg.pMsg,
                    g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen );                     /* 复制数据*/

        pucData[2] = (VOS_UINT8)(g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen - 1);     /* 填写消息长度 */

        ((SMS_LMM_DATA_REQ_STRU *)aucMsg)->stSmsMsg.ulSmsMsgSize
              = g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen + SMC_MSG_HDR_LEN;         /* 赋值消息长度 */

        PS_MEM_FREE(WUEPS_PID_SMS, g_SmcPsEnt.SmcMo.HoldRcvMsg.pMsg);                          /* 释放消息 */

        g_SmcPsEnt.SmcMo.HoldRcvMsg.ulMsgLen = 0;                               /* 清除消息长度 */

        g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg     = (VOS_VOID *)SMS_Malloc(usMsgLen);
        if ( VOS_NULL_PTR == g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg )
        {
            NAS_WARNING_LOG(WUEPS_PID_SMS, "NAS_SMS_ProcLmmEstCnf: Memory allocation fails" );
            return;
        }

        PS_MEM_CPY(g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg, aucMsg, usMsgLen);         /* 备份消息                                 */
        g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen = usMsgLen;                        /* 备份消息长度                             */
        /* 标记缓存的消息类型 */
        g_SmcPsEnt.SmcMo.enHoldSndMsgType    = NAS_SMS_HOLD_MSG_LTE_MO_RP_DATA;

        /* 启动定时器TC1M */
        NAS_INFO_LOG(WUEPS_PID_SMS, "NAS_SMS_ProcLmmEstCnf: TimerStart: TC1M" );
        if ( SMS_FALSE == SMS_TimerStart( SMC_TIMER_ID_PS_TC1M_MO ) )
        {
            NAS_INFO_LOG(WUEPS_PID_SMS, "NAS_SMS_ProcLmmEstCnf: TimerStart: TC1M Failed" );
            PS_MEM_FREE(WUEPS_PID_SMS, g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg);
            g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen = 0;                           /* 清除消息长度                             */
            g_SmcPsEnt.SmcMo.enHoldSndMsgType    = NAS_SMS_HOLD_MSG_BUTT;
            return;
        }

        g_SmcPsEnt.SmcMo.TimerInfo.ucExpireTimes = 0;

        /* 发送CP-DATA消息 */
        if ( VOS_FALSE == NAS_SMS_SndLmmCpDataReq(aucMsg, usMsgLen) )
        {
            NAS_INFO_LOG(WUEPS_PID_SMS, "NAS_SMS_ProcLmmEstCnf: Message allocation fails" );
            PS_MEM_FREE(WUEPS_PID_SMS, g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg);
            g_SmcPsEnt.SmcMo.HoldSndMsg.ulMsgLen = 0;
            g_SmcPsEnt.SmcMo.enHoldSndMsgType    = NAS_SMS_HOLD_MSG_BUTT;
            return;
        }

        g_SmcPsEnt.SmcMo.ucState = SMC_MO_WAIT_FOR_CP_ACK;                      /* 状态迁移                                 */

    }

    return;
}


VOS_VOID NAS_SMS_ProcLmmDataInd(
    VOS_VOID                           *pRcvMsg
)
{
    LMM_SMS_DATA_IND_STRU              *pstLmmDataInd;
    VOS_UINT8                           ucTi;

    pstLmmDataInd = (LMM_SMS_DATA_IND_STRU *)pRcvMsg;

    ucTi          = pstLmmDataInd->stSmsMsg.aucSmsMsg[0] >> 4;

    SMS_ReportN2MOtaMsg((NAS_MSG_STRU *)&pstLmmDataInd->stSmsMsg);

    /* 如果是MT过程 */
    if ((pstLmmDataInd->stSmsMsg.aucSmsMsg[0] >> 4) < NAS_SMS_MSG_SND_TO_SIDE_THAT_ORIGINATES_THE_TI)
    {
        /* 如果同时有两个MT过程 */
        if (SMC_MT_IDLE != g_SmcCsEnt.SmcMt.ucState)
        {
            /* 封装SMS_LMM_DATA_REQ_STRU结构,向网侧回复cp_err,调用NAS_SMS_SndLmmDataReq进行发送 */
            NAS_SMS_SndLmmDataReq(SMC_DATA_TYPE_CP_ERR, SMS_CP_ERR_CONGEST, ucTi);
        }
        else
        {
            /* 调用MT的处理过程 */
            SMC_RcvPsMtData(pstLmmDataInd->stSmsMsg.aucSmsMsg,
                            pstLmmDataInd->stSmsMsg.ulSmsMsgSize);
        }
    }
    else
    {
        /* 是MO过程，调用MO的处理过程 */
        SMC_RcvPsMoData(pstLmmDataInd->stSmsMsg.aucSmsMsg,
                        pstLmmDataInd->stSmsMsg.ulSmsMsgSize);
    }

    return;
}
VOS_VOID NAS_SMS_ProcLmmErrInd(
    VOS_VOID                           *pRcvMsg
)
{
    LMM_SMS_ERR_IND_STRU               *pstLmmSmsErrInd;

    pstLmmSmsErrInd = (LMM_SMS_ERR_IND_STRU *)pRcvMsg;

    SMC_ComPsMtErr(SMR_SMT_ERROR_EPS_ERROR_BEGIN | pstLmmSmsErrInd->enErrCause);    /* 调用PS域MT实体的处理 */

    SMC_ComPsMoErr(SMR_SMT_ERROR_EPS_ERROR_BEGIN | pstLmmSmsErrInd->enErrCause);    /* 调用PS域MO实体的处理 */

    return;
}


VOS_VOID NAS_SMS_SndLmmDataReq(
    VOS_UINT8                           ucType,
    VOS_UINT8                           ucCause,
    VOS_UINT8                           ucTi
)
{
    SMS_LMM_DATA_REQ_STRU              *pstSmsLmmDataReq;
    VOS_UINT32                          ulRet;

    /* 申请内存  */
    pstSmsLmmDataReq = (SMS_LMM_DATA_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_SMS,
                                     sizeof(SMS_LMM_DATA_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if ( VOS_NULL_PTR == pstSmsLmmDataReq )
    {
        NAS_WARNING_LOG(WUEPS_PID_SMS, "NAS_SMS_SndLmmDataReq():WARNING:PS_ALLOC_MSG FIAL");
        return;
    }

    PS_MEM_SET((VOS_INT8 *)pstSmsLmmDataReq + VOS_MSG_HEAD_LENGTH, 0,
               sizeof(SMS_LMM_DATA_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    /* 填写ti */
    pstSmsLmmDataReq->stSmsMsg.aucSmsMsg[0] = (VOS_UINT8)((ucTi ^ NAS_SMS_MSG_SND_TO_SIDE_THAT_ORIGINATES_THE_TI) << 4);
    pstSmsLmmDataReq->stSmsMsg.aucSmsMsg[0] |= NAS_PD_SMS;                       /* 填写PD  */
    pstSmsLmmDataReq->stSmsMsg.aucSmsMsg[1] = ucType;                            /* 填写消息类型 */

    if (SMC_DATA_TYPE_CP_ERR == ucType)
    {
        /* 数据类型是CP_ERROR */
        pstSmsLmmDataReq->stSmsMsg.ulSmsMsgSize = SMC_CP_ERR_LEN;
        pstSmsLmmDataReq->stSmsMsg.aucSmsMsg[2] = ucCause;
    }
    else
    {
        /* 数据类型是CP_ACK */
        pstSmsLmmDataReq->stSmsMsg.ulSmsMsgSize = SMC_CP_ACK_LEN;
    }

    pstSmsLmmDataReq->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstSmsLmmDataReq->ulReceiverPid   = PS_PID_MM;
    pstSmsLmmDataReq->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstSmsLmmDataReq->ulSenderPid     = WUEPS_PID_SMS;
    pstSmsLmmDataReq->ulLength        = sizeof(SMS_LMM_DATA_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstSmsLmmDataReq->ulMsgId         = ID_SMS_LMM_DATA_REQ;
    pstSmsLmmDataReq->ulOpId          = 0;

    /* 上报OTA消息 */
    SMC_ReportM2NOtaMsg((NAS_MSG_STRU *)&pstSmsLmmDataReq->stSmsMsg);

    /* 调用VOS发送原语 */
    ulRet = PS_SEND_MSG( WUEPS_PID_SMS, pstSmsLmmDataReq);

    if (VOS_OK != ulRet)
    {
        NAS_WARNING_LOG(WUEPS_PID_SMS, "NAS_SMS_SndLmmDataReq():WARNING:SEND LMM DATA REQ FIAL");
        return;
    }

    return;
}



VOS_UINT32 NAS_SMS_SndLmmCpDataReq(
    VOS_UINT8                          *pucSndMsg,
    VOS_UINT32                          ulMsgLen
)
{
    VOS_UINT8                          *pucCpDataMsg;
    LMM_SMS_MSG_STRU                   *pSmsMsg;

    /* 发送CP-DATA消息 */
    pucCpDataMsg = (VOS_UINT8* )PS_ALLOC_MSG( WUEPS_PID_SMS,
                                ulMsgLen - VOS_MSG_HEAD_LENGTH );

    if ( VOS_NULL_PTR == pucCpDataMsg )
    {
        NAS_INFO_LOG(WUEPS_PID_SMS, "NAS_SMS_SndLmmCpDataReq: Message allocation fails" );
        return VOS_FALSE;
    }

    PS_MEM_CPY(pucCpDataMsg, pucSndMsg, ulMsgLen);

    ((MSG_HEADER_STRU * )pucCpDataMsg)->ulSenderCpuId   = VOS_LOCAL_CPUID;
    ((MSG_HEADER_STRU * )pucCpDataMsg)->ulSenderPid     = WUEPS_PID_SMS;
    ((MSG_HEADER_STRU * )pucCpDataMsg)->ulReceiverCpuId = VOS_LOCAL_CPUID;
    ((MSG_HEADER_STRU * )pucCpDataMsg)->ulReceiverPid   = PS_PID_MM;
    ((MSG_HEADER_STRU * )pucCpDataMsg)->ulLength        = ulMsgLen - VOS_MSG_HEAD_LENGTH;
    ((MSG_HEADER_STRU * )pucCpDataMsg)->ulMsgName       = ID_SMS_LMM_DATA_REQ;

    pSmsMsg = &((SMS_LMM_DATA_REQ_STRU *)pucSndMsg)->stSmsMsg;
    SMC_ReportM2NOtaMsg((NAS_MSG_STRU *)pSmsMsg);

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_SMS, pucCpDataMsg))
    {
        NAS_WARNING_LOG(WUEPS_PID_SMS, "NAS_SMS_SndLmmCpDataReq():WARNING:SEND MSG FIAL");
    }

    return VOS_TRUE;

}
VOS_VOID NAS_SMS_SndLmmEstReq(
    VOS_UINT32                          ulEstCause
)
{
    VOS_UINT32                          ulRet;
    SMS_LMM_EST_REQ_STRU               *pstEstReqMsg;

    /* 申请内存  */
    pstEstReqMsg = (SMS_LMM_EST_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_SMS,
                                         sizeof(SMS_LMM_EST_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if( VOS_NULL_PTR == pstEstReqMsg )
    {
        NAS_ERROR_LOG(WUEPS_PID_SMS, "NAS_SMS_SndLmmEstReq():ERROR:Memory Alloc Error for pstEstReqMsg");
        return;
    }

    PS_MEM_SET((VOS_INT8*)pstEstReqMsg + VOS_MSG_HEAD_LENGTH, 0,
                     sizeof(SMS_LMM_EST_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    pstEstReqMsg->ulReceiverCpuId  = VOS_LOCAL_CPUID;
    pstEstReqMsg->ulReceiverPid    = PS_PID_MM;
    pstEstReqMsg->ulSenderCpuId    = VOS_LOCAL_CPUID;
    pstEstReqMsg->ulSenderPid      = WUEPS_PID_SMS;
    pstEstReqMsg->ulLength         = sizeof(SMS_LMM_EST_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstEstReqMsg->ulMsgId          = ID_SMS_LMM_EST_REQ;
    pstEstReqMsg->ulOpId           = 0;
    pstEstReqMsg->ulEstCause       = ulEstCause;

    /* 调用VOS发送原语 */
    ulRet = PS_SEND_MSG( WUEPS_PID_SMS, pstEstReqMsg );

    if( VOS_OK != ulRet )
    {
        NAS_WARNING_LOG(WUEPS_PID_SMS, "NAS_SMS_SndLmmEstReq():WARNING:SEND SMS LMM EST REQ FIAL");
        return;
    }

    return;
}
#endif

VOS_UINT32  NAS_SMS_IsPsDomainIdle(VOS_VOID)
{
    if ((SMC_MT_IDLE == g_SmcPsEnt.SmcMt.ucState)
     && (SMC_MO_IDLE == g_SmcPsEnt.SmcMo.ucState))
    {
        return VOS_TRUE;
    }
    else
    {
        return VOS_FALSE;
    }

}

VOS_VOID  NAS_SMS_ReSendLmmCpDataReq(
    VOS_UINT32                          ulResendMoFlg
)
{
    PMMSMS_UNITDATA_REQ_STRU           *pstHoldGuMsg    = VOS_NULL_PTR;
    SMS_LMM_DATA_REQ_STRU              *pstHoldLmmMsg   = VOS_NULL_PTR;
    SMS_LMM_DATA_REQ_STRU              *pstLmmMsg       = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLen;
    VOS_UINT32                          ulNasMsgLen;

    ulMsgLen    = 0;
    ulNasMsgLen = 0;

    /* 如果之前是在GU下发的CP_DATA,需要进行一下结构体转换,
       否则LTE下的消息结构体和GU下的消息结构体不一样，可能导致复位 */

    /* MO时 */
    if (VOS_TRUE == ulResendMoFlg)
    {
        if (NAS_SMS_HOLD_MSG_GU_MO_RP_DATA  == g_SmcPsEnt.SmcMo.enHoldSndMsgType)
        {
            pstHoldGuMsg    = (PMMSMS_UNITDATA_REQ_STRU *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg;
            ulNasMsgLen     = pstHoldGuMsg->SmsMsg.ulNasMsgSize;
            
            /* LTE下的消息结构体和GU下的消息结构体不一样，需要重新分配内存空间 */
            ulMsgLen    = sizeof(SMS_LMM_DATA_REQ_STRU) - 4 + ulNasMsgLen;
        
        
            pstLmmMsg = PS_MEM_ALLOC(WUEPS_PID_SMS, ulMsgLen);

            if (VOS_NULL_PTR == pstLmmMsg)
            {
                return;
            }
            
            PS_MEM_SET(pstLmmMsg, 0, ulMsgLen);
        
            pstLmmMsg->stSmsMsg.ulSmsMsgSize = ulNasMsgLen;
            PS_MEM_CPY(pstLmmMsg->stSmsMsg.aucSmsMsg, pstHoldGuMsg->SmsMsg.aucNasMsg, ulNasMsgLen);
        }
        else if (NAS_SMS_HOLD_MSG_LTE_MO_RP_DATA == g_SmcPsEnt.SmcMo.enHoldSndMsgType)
        {            
            pstHoldLmmMsg   = (SMS_LMM_DATA_REQ_STRU *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg;
            ulNasMsgLen     = pstHoldLmmMsg->stSmsMsg.ulSmsMsgSize;

            /* 重新分配内存空间 */
            ulMsgLen    = sizeof(SMS_LMM_DATA_REQ_STRU) - 4 + ulNasMsgLen;
            
            pstLmmMsg = PS_MEM_ALLOC(WUEPS_PID_SMS, ulMsgLen);
            
            if (VOS_NULL_PTR == pstLmmMsg)
            {
                return;
            }
            
            PS_MEM_SET(pstLmmMsg, 0, ulMsgLen);
        
            pstLmmMsg->stSmsMsg.ulSmsMsgSize = ulNasMsgLen;
            PS_MEM_CPY(pstLmmMsg->stSmsMsg.aucSmsMsg, pstHoldLmmMsg->stSmsMsg.aucSmsMsg, ulNasMsgLen);
        }
        else
        {
            return;
        }
        
    }
    /* MT时 */
    else
    {
        if (NAS_SMS_HOLD_MSG_GU_MT_RP_ACK  == g_SmcPsEnt.SmcMt.enHoldSndMsgType)
        {
            pstHoldGuMsg    = (PMMSMS_UNITDATA_REQ_STRU *)g_SmcPsEnt.SmcMt.HoldSndMsg.pMsg;
            ulNasMsgLen     = pstHoldGuMsg->SmsMsg.ulNasMsgSize;
            
            /* LTE下的消息结构体和GU下的消息结构体不一样，需要重新分配内存空间 */
            ulMsgLen    = sizeof(SMS_LMM_DATA_REQ_STRU) - 4 + ulNasMsgLen;
        
        
            pstLmmMsg = PS_MEM_ALLOC(WUEPS_PID_SMS, ulMsgLen);

            if (VOS_NULL_PTR == pstLmmMsg)
            {
                return;
            }
            
            PS_MEM_SET(pstLmmMsg, 0, ulMsgLen);
        
            pstLmmMsg->stSmsMsg.ulSmsMsgSize = ulNasMsgLen;
            PS_MEM_CPY(pstLmmMsg->stSmsMsg.aucSmsMsg, pstHoldGuMsg->SmsMsg.aucNasMsg, ulNasMsgLen);
        }
        else if (NAS_SMS_HOLD_MSG_LTE_MT_RP_ACK == g_SmcPsEnt.SmcMt.enHoldSndMsgType)
        {            
            pstHoldLmmMsg   = (SMS_LMM_DATA_REQ_STRU *)g_SmcPsEnt.SmcMt.HoldSndMsg.pMsg;
            ulNasMsgLen     = pstHoldLmmMsg->stSmsMsg.ulSmsMsgSize;

            /* 重新分配内存空间 */
            ulMsgLen    = sizeof(SMS_LMM_DATA_REQ_STRU) - 4 + ulNasMsgLen;
            
            pstLmmMsg = PS_MEM_ALLOC(WUEPS_PID_SMS, ulMsgLen);
            
            if (VOS_NULL_PTR == pstLmmMsg)
            {
                return;
            }
            
            PS_MEM_SET(pstLmmMsg, 0, ulMsgLen);
        
            pstLmmMsg->stSmsMsg.ulSmsMsgSize = ulNasMsgLen;
            PS_MEM_CPY(pstLmmMsg->stSmsMsg.aucSmsMsg, pstHoldLmmMsg->stSmsMsg.aucSmsMsg, ulNasMsgLen);
        }
        else
        {
            return;
        }
    }
    
    /* 发送CP-DATA消息 */
    NAS_SMS_SndLmmCpDataReq((VOS_UINT8 *)pstLmmMsg, ulMsgLen);
    
    /* 前面对pstLmmMsg有malloc，消息发送完成之后要释放 */
    PS_MEM_FREE(WUEPS_PID_SMS, pstLmmMsg);

    return;

}


VOS_VOID  NAS_SMS_ReSendGmmCpDataReq(
    VOS_UINT32                          ulResendMoFlg
)
{
    PMMSMS_UNITDATA_REQ_STRU           *pstHoldGuMsg    = VOS_NULL_PTR;
    SMS_LMM_DATA_REQ_STRU              *pstHoldLmmMsg   = VOS_NULL_PTR;
    PMMSMS_UNITDATA_REQ_STRU           *pstGuMsg        = VOS_NULL_PTR;
    VOS_UINT32                          ulMsgLen;
    VOS_UINT32                          ulNasMsgLen;

    /* 如果之前是在GU下发的CP_DATA,需要进行一下结构体转换,
       否则LTE下的消息结构体和GU下的消息结构体不一样，可能导致复位 */


    /* MO时 */
    if (VOS_TRUE == ulResendMoFlg)
    {
        if (NAS_SMS_HOLD_MSG_GU_MO_RP_DATA  == g_SmcPsEnt.SmcMo.enHoldSndMsgType)
        {
            pstHoldGuMsg    = (PMMSMS_UNITDATA_REQ_STRU *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg;
            ulNasMsgLen     = pstHoldGuMsg->SmsMsg.ulNasMsgSize;
            
            ulMsgLen    = sizeof(PMMSMS_UNITDATA_REQ_STRU) - 4 + ulNasMsgLen;
        
        
            pstGuMsg = PS_MEM_ALLOC(WUEPS_PID_SMS, ulMsgLen);

            if (VOS_NULL_PTR == pstGuMsg)
            {
                return;
            }
            
            PS_MEM_SET(pstGuMsg, 0, ulMsgLen);
        
            pstGuMsg->SmsMsg.ulNasMsgSize   = ulNasMsgLen;
            PS_MEM_CPY(pstGuMsg->SmsMsg.aucNasMsg, pstHoldGuMsg->SmsMsg.aucNasMsg, ulNasMsgLen);
        }
        else
        {            
            /* LTE下的消息结构体和GU下的消息结构体不一样，需要重新分配内存空间 */
            pstHoldLmmMsg   = (SMS_LMM_DATA_REQ_STRU *)g_SmcPsEnt.SmcMo.HoldSndMsg.pMsg;
            ulNasMsgLen     = pstHoldLmmMsg->stSmsMsg.ulSmsMsgSize;

            /* 重新分配内存空间 */
            ulMsgLen    = sizeof(PMMSMS_UNITDATA_REQ_STRU) - 4 + ulNasMsgLen;
            
            pstGuMsg = PS_MEM_ALLOC(WUEPS_PID_SMS, ulMsgLen);
            
            if (VOS_NULL_PTR == pstGuMsg)
            {
                return;
            }
            
            PS_MEM_SET(pstGuMsg, 0, ulMsgLen);
        
            pstGuMsg->SmsMsg.ulNasMsgSize   = ulNasMsgLen;
            PS_MEM_CPY(pstGuMsg->SmsMsg.aucNasMsg, pstHoldLmmMsg->stSmsMsg.aucSmsMsg, ulNasMsgLen);
        }
    }
    /* MT时 */
    else
    {
        if (NAS_SMS_HOLD_MSG_GU_MT_RP_ACK  == g_SmcPsEnt.SmcMt.enHoldSndMsgType)
        {
            pstHoldGuMsg    = (PMMSMS_UNITDATA_REQ_STRU *)g_SmcPsEnt.SmcMt.HoldSndMsg.pMsg;
            ulNasMsgLen     = pstHoldGuMsg->SmsMsg.ulNasMsgSize;
            
            ulMsgLen    = sizeof(PMMSMS_UNITDATA_REQ_STRU) - 4 + ulNasMsgLen;
        
        
            pstGuMsg = PS_MEM_ALLOC(WUEPS_PID_SMS, ulMsgLen);

            if (VOS_NULL_PTR == pstGuMsg)
            {
                return;
            }
            
            PS_MEM_SET(pstGuMsg, 0, ulMsgLen);
        
            pstGuMsg->SmsMsg.ulNasMsgSize   = ulNasMsgLen;
            PS_MEM_CPY(pstGuMsg->SmsMsg.aucNasMsg, pstHoldGuMsg->SmsMsg.aucNasMsg, ulNasMsgLen);
        }
        else
        {            
            /* LTE下的消息结构体和GU下的消息结构体不一样，需要重新分配内存空间 */
            pstHoldLmmMsg   = (SMS_LMM_DATA_REQ_STRU *)g_SmcPsEnt.SmcMt.HoldSndMsg.pMsg;
            ulNasMsgLen     = pstHoldLmmMsg->stSmsMsg.ulSmsMsgSize;

            /* 重新分配内存空间 */
            ulMsgLen    = sizeof(PMMSMS_UNITDATA_REQ_STRU) - 4 + ulNasMsgLen;
            
            pstGuMsg = PS_MEM_ALLOC(WUEPS_PID_SMS, ulMsgLen);
            
            if (VOS_NULL_PTR == pstGuMsg)
            {
                return;
            }
            
            PS_MEM_SET(pstGuMsg, 0, ulMsgLen);
        
            pstGuMsg->SmsMsg.ulNasMsgSize   = ulNasMsgLen;
            PS_MEM_CPY(pstGuMsg->SmsMsg.aucNasMsg, pstHoldLmmMsg->stSmsMsg.aucSmsMsg, ulNasMsgLen);
        }

    }
    
    SMC_SndGmmMsg((VOS_UINT8 *)pstGuMsg,
                  PMMSMS_UNITDATA_REQ,
                  (VOS_UINT16)ulMsgLen) ;             /* 重发消息                                 */
    
    /* 前面对pstGuMsg有malloc，消息发送完成之后要释放 */
    PS_MEM_FREE(WUEPS_PID_SMS, pstGuMsg);

    return;

}


/*lint -restore */

#ifdef  __cplusplus
  #if  __cplusplus
  }
  #endif
#endif


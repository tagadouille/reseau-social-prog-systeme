#ifndef PROTOCOL_H
#define PROTOCOL_H

#define CQ_INSCRI 0x01
#define CQ_REP_INSCRI 0x02
#define CQ_NEW_GRP 0x03
#define CQ_REP_NEW_GRP 0x04
#define CQ_INV_GRP 0x05
#define CQ_INV_ATT 0x06
#define CQ_LIST_INV 0x07
#define CQ_REP_INV 0x08
#define CQ_GRP_REJ 0x09
#define CQ_QUIT_GRP 0x18
#define CQ_LIST_MB 0x0a
#define CQ_REP_LIST_MB 0x0b
#define CQ_BILLET 0x0c
#define CQ_REP_SERV_BILL 0x0d
#define CQ_REP_BILL 0x0e
#define CQ_REP_SERV_REP_BILL 0x0f
#define CQ_LIST_LAST_BILL 0x10
#define CQ_REP_LIST_BILL 0x11

#define CQ_ERR 0x1F

#define CQ_NOTIF_REP 0x12
#define CQ_NOTIF_ACCEPT 0x13
#define CQ_NOTIF_QUIT_GRP 0x14
#define CQ_NOTIF_CLOSE_GRP 0x15
#define CQ_NOTIF_RECV_INV 0x16
#define CQ_NOTIF_BILLET 0x17

#endif

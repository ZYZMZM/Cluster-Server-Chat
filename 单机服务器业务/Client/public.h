#pragma once

/*
公共头文件，统一服务器和客户端的常量定义
*/
const int MSG_LOGIN = 0; // login msg
const int MSG_LOGIN_ACK = 1; // login ack msg
const int MSG_REG = 2; // register msg
const int MSG_REG_ACK = 3; // register ack msg
const int MSG_ADD_FRIEND = 4; // add friend msg
const int MSG_ADD_FRIEND_ACK = 5; // add friend ack msg
const int MSG_ADD_GROUP = 6; // add group msg
const int MSG_ADD_GROUP_ACK = 7; // add group ack msg
const int MSG_ONE_CHAT = 8; // one to one chat msg
const int MSG_ONE_CHAT_ACK = 9; // one to one chat ack msg
const int MSG_LOGINOUT = 10; // logout msg
const int MSG_LOGINOUT_ACK = 11; // logout msg ack
const int MSG_OFFMSG = 12; // offline msg
const int MSG_OFFMSG_ACK = 13; // offline msg ack
const int MSG_GROUP_CHAT = 14; // group chat msg
const int MSG_GROUP_CHAT_ACK = 15; // group chat ack msg
const int MSG_CRE_GROUP = 16; // create group  msg
const int MSG_CRE_GROUP_ACK = 17; // create group ack msg

const int ACK_SUCCESS = 100; // msg process ok
const int ACK_ERROR = 101; // msg process err
const int ACK_NOT_FRI = 102; // not friend
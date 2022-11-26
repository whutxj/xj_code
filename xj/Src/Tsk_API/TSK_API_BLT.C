
//YIYUAN ÒÆÔ¶Ä£¿é
static uint8 BLT_Init(void)
{

  char cmd[64] = {0};
  char cmd_2[64] = {0};
  char ip[54] = {0};

  uint32 port = 0;
  int ReConnectCount = 0;

  Get_Server_Ip((U8*)ip);

  port = Get_Server_Port();

  dbg_printf(1,"in Yiyuan_4G!");
  while(ReConnectCount < 3)
  {
  if(AT_Cmd_Parse("+++", "OK",NULL, 1000)== FALSE)
    {

      //ReConnectCount++;
      osDelay(1000);
      //continue;
    }
  if(AT_Cmd_Parse("ATE0\r", "OK",NULL, 500)== FALSE)
    {
      osDelay(1000);
      ReConnectCount++;
      dbg_printf(1,"ATE0 not ok!");
      continue;
    }
   if(AT_Cmd_Parse("AT+QICSGP=1,1,\"\",\"\",\"\",1\r", "OK",NULL, 500)== FALSE)
    {
     osDelay(1000);
      ReConnectCount++;
     dbg_printf(1,"AT+QICSGP not ok!");
      continue;
    }
   if(AT_Cmd_Parse("AT+QIDEACT=1\r", "OK",NULL, 500)== FALSE)
    {
      osDelay(1000);
      ReConnectCount++;
      dbg_printf(1,"AT+QIDEACT not ok!");
      continue;
    }
  if(AT_Cmd_Parse("AT+QIACT=1\r", "OK",NULL, 1000)== FALSE)
    {
      osDelay(1000);
    }
    osDelay(200);
    memset(cmd,0,sizeof(cmd));
    memset(cmd_2,0,sizeof(cmd_2));

    sprintf(cmd_2,"%s","\"");
    sprintf(cmd_2,"%s%s",cmd_2,ip);
    sprintf(cmd_2,"%s%s",cmd_2,"\"");
    sprintf(cmd,"AT+QIOPEN=1,1,%s,%s,%d,0,2\r","\"TCP\"",cmd_2,port);
    dbg_printf(1,"cmd =%s",cmd);
    //if(AT_Cmd_Parse("AT+QIOPEN=1,1,\"TCP\",\"zijieyun.com\",1038,0,2\r", "CONNECT",NULL, 500)== FALSE)
    if(AT_Cmd_Parse(cmd, "CONNECT",NULL, 10000)== FALSE)
    {
      ReConnectCount++;
      osDelay(1000);
      dbg_printf(1,"TCP CONNECT not ok!");
      continue;
    }
    osDelay(10000);
    dbg_printf(1,"SET YiYuan 4g OK");
    return TRUE;
  }
  return FALSE;
}

//³­ tsk_api_card.c
//blt search 
// blt  read 
//blt write
//blt auth
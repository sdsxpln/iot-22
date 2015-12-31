#ifndef _CLI_H_
#define _CLI_H_


typedef void (*CliCmdFunc) ( s32, s8 ** );


typedef struct CLICmds_st
{
        const s8           *Cmd;
        CliCmdFunc          CmdHandler;
        const s8           *CmdUsage;
        
} CLICmds, *PCLICmds;




s32  Cli_RunCmd(s8 *CmdBuffer);
void Cli_Init(s32 argc, s8 *argv[]);
void Cli_Start(void);
void Cli_Task( void *args );

#endif /* _CLI_H_ */


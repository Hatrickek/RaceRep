#include "DiscordRPC.h"
#include "accmem.h"
#include "overlays.h"
#include <string>
void UpdateGraphicsPF(void);
void UpdateStaticPF(void);
char* wchar_to_char(const wchar_t* pwchar);
const char* sessionType();
const char* gameStatus();
SPageFileGraphic* m_pf_grp;
SPageFileStatic* m_pf_stc;
bool rpcInitalized = false;
void DiscordRPC::Initialize()
{
    UpdateGraphicsPF();
    UpdateStaticPageFile();
    m_pf_grp = GetGraphicsPageFile();
    m_pf_stc = GetStaticPageFile();
    DiscordEventHandlers handler;
	memset(&handler, 0, sizeof(handler));
	Discord_Initialize(DISCORD_APP_ID, &handler, 1, NULL);
    rpcInitalized = true;
}
void DiscordRPC::Update()
{
    if (!rpcInitalized) return;
    UpdateGraphicsPF();
    UpdateStaticPageFile();
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    discordPresence.state = sessionType();
    discordPresence.details = gameStatus();
    /*discordPresence.startTimestamp = NULL;
    if(m_pf_grp->sessionTimeLeft != NULL) discordPresence.endTimestamp = (int64_t)m_pf_grp->sessionTimeLeft;*/
    char* trackname = wchar_to_char(m_pf_stc->track);
    discordPresence.largeImageText = trackname;
    //free(trackname);
    char* carname = wchar_to_char(m_pf_stc->carModel);
    discordPresence.smallImageText = carname;
    //free(carname);
    discordPresence.largeImageKey = "aclogo";
    discordPresence.smallImageKey = "racecarlogo";
    Discord_UpdatePresence(&discordPresence);
}
const char* sessionType()
{
    const char* value;
    switch (m_pf_grp->session)
    {
    case 0:
        value = "Practice";
        break;
    case 1:
        value = "Qualify";
        break;
    case 2:
        value = "Race";
        break;
    case 3:
        value = "Hotlap";
        break;
    default:
        value = "In Menu";
        break;
    }
    return value;
}
const char* gameStatus()
{
    const char* value{};
    switch (m_pf_grp->status){
    case 0:
		value = "In Menu";
        break;
    case 1:
		value = "Replay";
        break;
    case 2:
		value = "Playing";
        break;
    case 3:
		value = "Paused";
        break;
    default:
		value = "In Menu";
        break;
    }
    return value;
}
char* wchar_to_char(const wchar_t* pwchar)
{
    int currentCharIndex = 0;
    char currentChar = pwchar[currentCharIndex];
    
    while (currentChar != '\0')
    {
        currentCharIndex++;
        currentChar = pwchar[currentCharIndex];
    }
    
    const int charCount = currentCharIndex + 1;
    
    // allocate a new block of memory size char (1 byte) instead of wide char (2 bytes)
    char* newchar = (char*)malloc(sizeof(char) * charCount);
    
    for (int i = 0; i < charCount; i++)
    {
        // convert to char (1 byte)
        char character = pwchar[i];
        
		if(newchar != NULL)
            *newchar = character;
    
        newchar += sizeof(char);
    
    }
    newchar += '\0';
    
    newchar -= (sizeof(char) * charCount);
    
    return newchar;
}
void UpdateGraphicsPF()
{
    if (m_pf_grp == NULL)
    {
        m_pf_grp = (SPageFileGraphic*)m_graphics.mapFileBuffer;
    }
}
void UpdateStaticPF()
{
    if (m_pf_stc == NULL)
    {
        m_pf_stc = (SPageFileStatic*)m_static.mapFileBuffer;
    }
}
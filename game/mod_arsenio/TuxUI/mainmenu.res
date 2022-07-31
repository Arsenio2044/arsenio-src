"MainMenu"
{	
	"ResumeGame"
	{
		"text"			"#GameUI2_ResumeGame"
		"description"	"#GameUI2_ResumeGameDescription"
		"command"		"cvar gamemenucommand resumegame"
		"priority"		"5"
		"specifics"		"ingame"
	}
	
	"SaveGame"
	{
		"text"			"#GameUI2_SaveGame"
		"description"	"#GameUI2_SaveGameDescription"
		"command"		"cvar gamemenucommand opensavegamedialog"
		"priority"		"4"
		"specifics"		"ingame"
	}

	
	"NewGame"
	{
		"text"			"#GameUI2_NewGame"
		"description"	"#GameUI2_NewGameDescription"
		"command"		"cvar gamemenucommand opennewgamedialog"
		"priority"		"4"
		"specifics"		"mainmenu"
	}

	"DeathMatch"
	{
		"text"			"DEATHMATCH"
		"description"	"FIGHT BOTS WITH KILLSTREAKS AND UNIQUE ABILITIES."
		"command"		"cvar deathmatch"
		"priority"		"3"
		"specifics"		"mainmenu"
	}
	
	
	"Options"
	{
		"text"			"#GameUI2_Options"
		"description"	"#GameUI2_OptionsDescription"
		"command"		"cvar gamemenucommand openoptionsdialog"
	//	"command"		"cvar gameui2_openoptionsdialog"
		"priority"		"2"
		"specifics"		"shared"
	}

	"Quit"
	{
		"text"			"#GameUI2_Quit"
		"description"	"#GameUI2_QuitDescription"
		"command"		"cvar gamemenucommand quit"
	//	"command"		"cvar gameui2_openquitgamedialog"
		"priority"		"1"
		"specifics"		"mainmenu"
	}

	"Menu"
	{
		"text"			"#GameUI2_Menu"
		"description"	"#GameUI2_MenuDescription"
		"command"		"cvar startupmenu"
	//	"command"		"cvar gameui2_openquitgamedialog"
		"priority"		"1"
		"specifics"		"ingame"
	}
}
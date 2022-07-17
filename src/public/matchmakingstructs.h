#ifndef MATCHMAKINGSTRUCTS_H
#define MATCHMAKINGSTRUCTS_H
struct MM_QOS_t
{
	int nPingMsMin;		// Minimum round-trip time in ms
	int nPingMsMed;		// Median round-trip time in ms
	float flBwUpKbs;	// Bandwidth upstream in kilobytes/s
	float flBwDnKbs;	// Bandwidth downstream in kilobytes/s
	float flLoss;		// Average packet loss in percents
};
#endif
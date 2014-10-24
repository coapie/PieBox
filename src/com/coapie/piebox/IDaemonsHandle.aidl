package com.coapie.piebox;

interface IDaemonsHandle{
	int setRepoParam(String rdir, int port);
	int getRepoStatus();
	int startRepo();
	int stopRepo();
	int restartRepo();
	int cleanRepo();
}
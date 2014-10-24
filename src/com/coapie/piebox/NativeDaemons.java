package com.coapie.piebox;

public class NativeDaemons {
	/*
	** Repository native daemon interface 
	*/ 
	private native int setNativeRepoParam(String rdir, short port);
	public int setRepoParam(String rdir, short port){
		return setNativeRepoParam(rdir, port);
	}
	
	private native int getNativeRepoStatus();
	public int getRepoStatus(){
		return getNativeRepoStatus();
	}
	
	private native int startNativeRepo();
	public int startRepo(){
		return startNativeRepo();
	}
	
	private native int stopNativeRepo();
	public int stopRepo(){
		return stopRepo();
	}
	
	private native int restartNativeRepo();
	public int restartRepo(){
		return restartNativeRepo();
	}
	
	private native int cleanNativeRepo();
	public int cleanRepo(){
		return cleanNativeRepo();
	}
	
	/*
	** transmission native daemon interface 
	*/
	private native int setNativeTransParam(String rdir);
	public int setTransParam(String rdir){
		return setNativeTransParam(rdir);
	}
	
	private native int getNativeTransStatus();
	public int getTransStatus(){
		return getNativeTransStatus();
	}
	
	private native int startNativeTrans();
	public int startTrans(){
		return startNativeTrans();
	}
	
	private native int stopNativeTrans();
	public int stopTrans(){
		return stopNativeTrans();
	}
	
	private native int restartNativeTrans();
	public int restartTrans(){
		return restartNativeTrans();
	}
}

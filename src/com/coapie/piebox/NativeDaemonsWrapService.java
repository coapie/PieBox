package com.coapie.piebox;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

public class NativeDaemonsWrapService extends Service {
	private NativeDaemons mDaemons;
	private final IDaemonsHandle.Stub mBinder = new IDaemonsHandle.Stub() {
		
		@Override
		public int stopRepo() throws RemoteException {
			return mDaemons.stopRepo();
		}
		
		@Override
		public int startRepo() throws RemoteException {
			return mDaemons.startRepo();
		}
		
		@Override
		public int setRepoParam(String rdir, int port) throws RemoteException {
			return mDaemons.setRepoParam(rdir, (short)port);
		}
		
		@Override
		public int restartRepo() throws RemoteException {
			return mDaemons.restartRepo();
		}
		
		@Override
		public int getRepoStatus() throws RemoteException {
			return mDaemons.getRepoStatus();
		}
		
		@Override
		public int cleanRepo() throws RemoteException {
			return mDaemons.cleanRepo();
		}
	};

	
	@Override
	public void onCreate(){
		mDaemons = new NativeDaemons();
	}
    
    public void onStart(Intent intent, int startId){  
    }  
      
    public IBinder onBind(Intent t){  
        return mBinder;  
    }  
      
    public void onDestroy(){  
        super.onDestroy();  
    }  
      
    public boolean onUnbind(Intent intent){  
        return super.onUnbind(intent);  
    }  
      
    public void onRebind(Intent intent){  
        super.onRebind(intent);  
    }  
    
    static {
    	System.loadLibrary("daemons");
    } 
}

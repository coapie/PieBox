package com.coapie.piebox;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import android.content.Context;
import android.net.wifi.SupplicantState;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.RemoteException;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;


public class SlidingFragment extends Fragment {

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
		return inflater.inflate(R.layout.sliding_fragment, null);
	}

	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		
		View main = getView();
		
        initListView(main);
        
        ImageView iv = (ImageView)main.findViewById(R.id.user_image);
        iv.setImageResource(R.drawable.user);
        
        TextView tv = (TextView)main.findViewById(R.id.user_text);
        tv.setText("PieBox");
        {
        	File fs = getActivity().getApplicationContext().getDir("", 0);
        	fs = new File(fs.getAbsoluteFile() + "/PieBox0");
        	boolean rc = fs.mkdirs();
        	Log.e("==getDir==", fs.getAbsolutePath() + rc);
        	
        	fs = Environment.getDataDirectory();
        	fs = new File(fs.getAbsoluteFile() + "/PieBox1");
        	rc = fs.mkdirs();
        	Log.e("==getDataDirectory==", fs.getAbsolutePath() + rc);
        	
        	fs = Environment.getExternalStorageDirectory();
        	fs = new File(fs.getAbsoluteFile() + "/PieBox2");
        	rc = fs.mkdirs();
        	Log.e("==getExternalStorageDirectory==", fs.getAbsolutePath() + rc);
        	
        	fs = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_MOVIES);
        	fs = new File(fs.getAbsoluteFile() + "/PieBox3");
        	rc = fs.mkdirs();
        	Log.e("==getExternalStoragePublicDirectory==", fs.getAbsolutePath() + rc);
        }
        
        Button btnRestart = (Button)main.findViewById(R.id.develop_restart);
        btnRestart.setOnClickListener(new View.OnClickListener() {
        	public void onClick(View v) {
        		MainActivity act = (MainActivity)getActivity();
        		IDaemonsHandle dh = act.getDaemonsHandle();
        		if(dh != null){
        			try {
						int rc = dh.restartRepo();
	            		Toast.makeText(act.getBaseContext(), "restart repo return " + rc, Toast.LENGTH_LONG).show();
					} catch (RemoteException e) {
	            		Toast.makeText(act.getBaseContext(), "restart repo fail", Toast.LENGTH_LONG).show();
					}
        		}
                 
        	}
        });
        

        Button clean = (Button)main.findViewById(R.id.develop_clean);
        clean.setOnClickListener(new View.OnClickListener() {
        	public void onClick(View v) {
        		MainActivity act = (MainActivity)getActivity();
        		IDaemonsHandle dh = act.getDaemonsHandle();
        		if(dh != null){
        			try {
						int rc = dh.cleanRepo();
	            		Toast.makeText(act.getBaseContext(), "clean repo return " + rc, Toast.LENGTH_LONG).show();
					} catch (RemoteException e) {
	            		Toast.makeText(act.getBaseContext(), "clean repo fail", Toast.LENGTH_LONG).show();
					}
        		}
                 
        	}
        });
        
        tv = (TextView)main.findViewById(R.id.develop_address);
        tv.setText("Wifi:" + getWifiIPAddress());
	}
	
	private String getWifiIPAddress() {
		WifiManager wifiManager = (WifiManager) getActivity().getSystemService(Context.WIFI_SERVICE);
		if(!wifiManager.isWifiEnabled()){
			return "not enabled!";
		}
		
		WifiInfo wifiInfo = wifiManager.getConnectionInfo();
		//if(wifiInfo.getSupplicantState() != SupplicantState.ASSOCIATED){
		//	return "not connected!";
		//}
		
	    return intToIp(wifiInfo.getIpAddress());
	}

	private String intToIp(int i) {
		return (i & 0xFF) + "." + ((i >> 8) & 0xFF) + "." + ((i >> 16) & 0xFF)
				+ "." + (i >> 24 & 0xFF);
	}

	private void initListView(View container){  
        ListView lv = (ListView) container.findViewById(R.id.sliding_listview);
        
        ArrayList<HashMap<String, Object>> listItem = new ArrayList<HashMap<String, Object>>();  
        
        HashMap<String, Object> home = new HashMap<String, Object>();    
        home.put("menuImage", R.drawable.home); 
        home.put("menuTitle", this.getResources().getString(R.string.sliding_menu_Home));
        listItem.add(home);  
        
        HashMap<String, Object> iarch = new HashMap<String, Object>();  
        iarch.put("menuImage", R.drawable.iarch); 
        iarch.put("menuTitle", this.getResources().getString(R.string.sliding_menu_iArch));
        listItem.add(iarch);  
        
        HashMap<String, Object> download = new HashMap<String, Object>();  
        download.put("menuImage", R.drawable.download); 
        download.put("menuTitle", this.getResources().getString(R.string.sliding_menu_Download));
        listItem.add(download);  
        
        HashMap<String, Object> groups = new HashMap<String, Object>();  
        groups.put("menuImage", R.drawable.groups); 
        groups.put("menuTitle", this.getResources().getString(R.string.sliding_menu_Groups));
        listItem.add(groups); 
        
        HashMap<String, Object> dis = new HashMap<String, Object>();  
        dis.put("menuImage", R.drawable.discovery); 
        dis.put("menuTitle", this.getResources().getString(R.string.sliding_menu_Discovery)); 
        listItem.add(dis);  
        
        HashMap<String, Object> conf = new HashMap<String, Object>();  
        conf.put("menuImage", R.drawable.configure); 
        conf.put("menuTitle", this.getResources().getString(R.string.sliding_menu_Configure)); 
        listItem.add(conf);  
        
        
        SimpleAdapter listItemAdapter = new SimpleAdapter (this.getActivity().getBaseContext(),
        		listItem,
        		R.layout.sliding_view,
        		new String[] {"menuImage","menuTitle"},
        		new int[] {R.id.menu_image, R.id.menu_text});  
         
        lv.setAdapter(listItemAdapter);
        

        lv.setOnItemClickListener(new OnItemClickListener() {  
            @Override  
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        		MainActivity act = (MainActivity)getActivity();
        		act.switchContent(position);            		
//            		ListView lvp = (ListView)parent;  
//            		HashMap<String, Object> map = (HashMap<String, Object>) lvp.getItemAtPosition(position);  
//            		String title = (String)map.get("menuTitle");  
//            		Toast.makeText(act.getBaseContext(), title + position +"--" + id,Toast.LENGTH_LONG).show();  
             }  
        }); 
	}
}

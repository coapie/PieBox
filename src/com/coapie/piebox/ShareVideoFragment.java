package com.coapie.piebox;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

public class ShareVideoFragment extends Fragment {
	private ListView mLV;

	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
		View main = inflater.inflate(R.layout.share_folder_layout, null);		
		mLV = (ListView)main.findViewById(R.id.share_folder_listview);
		
		return main;
	}

}

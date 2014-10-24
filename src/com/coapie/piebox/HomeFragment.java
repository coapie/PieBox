package com.coapie.piebox;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.GridView;
import android.widget.SimpleAdapter;

public class HomeFragment extends Fragment {
	/*
	private CoapieButton mNew;
	private CoapieButton mCapacity;
	private CoapieButton mShare;
	private CoapieButton mDownload;
	
	private CoapieButton mDiscovery;
	private CoapieButton mGroups;
	private CoapieButton mMessage;
	private CoapieButton mConfigure;
	*/
	
	private static final int mImages[] = {R.drawable.home_button_new, R.drawable.home_button_share,
		R.drawable.home_button_download, R.drawable.home_button_discovery,R.drawable.home_button_groups,
		R.drawable.home_button_configure};
	
	private static final int mTitles[] = {R.string.home_button_new,R.string.home_button_share,
		R.string.home_button_download, R.string.home_button_discovery,R.string.home_button_groups,
		R.string.home_button_configure};
	
	private static final int mItems = 6;
	
	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
		return inflater.inflate(R.layout.home_fragment, null);		
	}


	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		
		View main = getView();
		

		List<Map<String, Object>> items = new ArrayList<Map<String,Object>>();
        for (int i = 0; i < mItems; i++) {
            Map<String, Object> item = new HashMap<String, Object>();
            item.put("ButtonImage", mImages[i]);
            item.put("ButtonTitle", getResources().getString(mTitles[i]));
            items.add(item);
        }
        
        SimpleAdapter adapter = new SimpleAdapter(getActivity().getBaseContext(), 
        		items, R.layout.coapie_button, new String[]{"ButtonImage", "ButtonTitle"}, 
        		new int[]{R.id.coapie_button_image, R.id.coapie_button_title}); 
        
        GridView gridview = (GridView)main.findViewById(R.id.home_gridview_button);
        gridview.setAdapter(adapter);
        
/*		
		mNew = (CoapieButton)main.findViewById(R.id.home_button_new);
		mNew.setContent(R.drawable.home_button_new, R.string.home_button_new);
		
		mCapacity = (CoapieButton)main.findViewById(R.id.home_button_capacity);
		mCapacity.setContent(R.drawable.home_button_capacity, R.string.home_button_capacity);
		
		mShare = (CoapieButton)main.findViewById(R.id.home_button_share);
		mShare.setContent(R.drawable.home_button_share, R.string.home_button_share);
		
		mDownload = (CoapieButton)main.findViewById(R.id.home_button_download);
		mDownload.setContent(R.drawable.home_button_download, R.string.home_button_download);
		
		mDiscovery = (CoapieButton)main.findViewById(R.id.home_button_discovery);
		mDiscovery.setContent(R.drawable.home_button_discovery, R.string.home_button_discovery);
		
		mGroups = (CoapieButton)main.findViewById(R.id.home_button_groups);
		mGroups.setContent(R.drawable.home_button_groups, R.string.home_button_groups);
		
		mMessage = (CoapieButton)main.findViewById(R.id.home_button_message);
		mMessage.setContent(R.drawable.home_button_message, R.string.home_button_message);
		
		mConfigure = (CoapieButton)main.findViewById(R.id.home_button_configure);
		mConfigure.setContent(R.drawable.home_button_configure, R.string.home_button_configure);	
*/	
	}
}

package com.coapie.piebox;

import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTabHost;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TextView;
import android.widget.TabHost.TabSpec;
 
public class ShareFragment extends Fragment {
	private FragmentTabHost mTabHost;
	
	private int mImageViewArray[] = {R.drawable.tabhost_video,R.drawable.tabhost_music,
			R.drawable.tabhost_photo, R.drawable.tabhost_docs, R.drawable.tabhost_more};

	private int mTextviewArray[] = {R.string.tabhost_video,R.string.tabhost_music,
			R.string.tabhost_photo, R.string.tabhost_docs, R.string.tabhost_more};

	private Class mFragmentArray[] = {ShareVideoFragment.class, ShareMusicFragment.class, SharePhotoFragment.class,
			ShareDocsFragment.class, ShareMoreFragment.class};

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState){
		View main = inflater.inflate(R.layout.share_fragment, null);		
	
		mTabHost = (FragmentTabHost)main.findViewById(android.R.id.tabhost);
        mTabHost.setup(getActivity(), getChildFragmentManager(), R.id.realtabcontent);
        
        for(int i = 0; i < 5; i++){
        	TabSpec tabSpec = mTabHost.newTabSpec(getString(mTextviewArray[i])).setIndicator(setTabView(i));
        	mTabHost.addTab(tabSpec, mFragmentArray[i], null);
        }
        
        mTabHost.setOnTabChangedListener(new OnTabChangeListener() {
            @Override
            public void onTabChanged(String tabId) {

                for (int i = 0; i < mTabHost.getTabWidget().getChildCount(); i++) {
                    mTabHost.getTabWidget().getChildAt(i)
                            .setBackgroundColor(Color.parseColor("#FFFFFF"));
                }

                mTabHost.getTabWidget().getChildAt(mTabHost.getCurrentTab())
                        .setBackgroundColor(Color.parseColor("#DCDCDC"));

            }
        });

        mTabHost.getTabWidget().getChildAt(mTabHost.getCurrentTab())
                .setBackgroundColor(Color.parseColor("#DCDCDC"));
        return main;
	}


	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
	}
	
	 
	@Override
	 public void onDestroyView() {
		super.onDestroyView();
	    mTabHost = null;
	 }
	
	private View setTabView(int index){
		LayoutInflater inflater = getActivity().getLayoutInflater();
		View view = inflater.inflate(R.layout.tabhost_layout, null);
		
		ImageView imageView = (ImageView) view.findViewById(R.id.tabhost_image);
		imageView.setImageResource(mImageViewArray[index]);
		
		TextView textView = (TextView) view.findViewById(R.id.tabhost_title);
		textView.setText(getString(mTextviewArray[index]));
		return view;
	}
}

package com.coapie.piebox;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class CoapieButton extends RelativeLayout {
	private TextView mTitle;
	private ImageView mImage;
	
	public CoapieButton(Context context, AttributeSet attrs) {
		super(context, attrs);
		
		LayoutInflater inflater = (LayoutInflater)context.getSystemService
			      (Context.LAYOUT_INFLATER_SERVICE);
		inflater.inflate(R.layout.coapie_button, this, true);
		
		mTitle = (TextView) findViewById(R.id.coapie_button_title);
		mImage = (ImageView) findViewById(R.id.coapie_button_image);
	}
	
	public void setContent(int imageResId, int titleResId){
		mImage.setImageResource(imageResId);
		mTitle.setText(titleResId);
	}

//	@Override
//	protected void onLayout(boolean changed, int l, int t, int r, int b) {
//		super.layout(l, t, r, b);
//	}
}

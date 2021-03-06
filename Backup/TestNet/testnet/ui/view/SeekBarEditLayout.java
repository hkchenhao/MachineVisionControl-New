package com.hanyu.hust.testnet.ui.view;

import android.content.Context;
import android.text.Editable;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

public class SeekBarEditLayout extends LinearLayout
{
	public interface ActionListener
	{
		void onChange(SeekBar seekBar);
	}
	
	private ActionListener listener;
	
	Context context;
	
	SeekBar seekBar;
	
	EditText editText;

	private boolean textChanged = false;					// 标志editText是否已经是最新的

	public SeekBarEditLayout(Context context)
	{
		this(context, null);
	}

	public SeekBarEditLayout(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		this.context = context;
		setOrientation(LinearLayout.HORIZONTAL);
		init_widget();
	}
	private void init_widget()
	{
		seekBar = new SeekBar(context);
		editText = new EditText(context);

		LayoutParams params = new LayoutParams(0, LayoutParams.MATCH_PARENT,
				3.5F);
		seekBar.setLayoutParams(params);
		seekBar.setMax(1000);
		seekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener()
		{
			@Override
			public void onStopTrackingTouch(SeekBar seekBar)
			{
				if (listener !=  null)
					listener.onChange(seekBar);
			}

			@Override
			public void onStartTrackingTouch(SeekBar seekBar)
			{				
			}

			@Override
			public void onProgressChanged(SeekBar seekBar, int progress,
                                          boolean fromUser)
			{
				if (!textChanged)
				{
					editText.setText(progress + "");
				}
				textChanged = false;
			}
		});

		params = new LayoutParams(0, LayoutParams.MATCH_PARENT, 1.4F);
		editText.setLayoutParams(params);
		editText.setInputType(InputType.TYPE_CLASS_NUMBER);
		editText.addTextChangedListener(new TextWatcher()
		{
			@Override
			public void onTextChanged(CharSequence s, int start, int before,
                                      int count)
			{
			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count,
                                          int after)
			{
				
			}

			@Override
			public void afterTextChanged(Editable s)
			{
				textChanged = true;
				if (seekBar != null)
				{
					if (TextUtils.isEmpty(s.toString()))
					{
						seekBar.setProgress(0);
					}
					else
					{
						try
						{
							seekBar.setProgress(Integer.valueOf(s.toString()));
						} catch (NumberFormatException e)
						{
							Log.e("MC", e.getMessage());
						}
					}
				}
			}
		});
		editText.setOnEditorActionListener(new OnEditorActionListener()
		{
			@Override
			public boolean onEditorAction(TextView v, int actionId,
                                          KeyEvent event)
			{
				if (seekBar != null)
				{
					seekBar.setProgress(Integer.valueOf(v.getText().toString()));
				}
				return false;
			}
		});

		addView(seekBar);
		addView(editText);
	}
	
	public void setonActionListener(ActionListener listener)	
	{
		this.listener = listener;
	}
		
	/**
	 * 获取控件的当前值
	 * @return 当前控件的值 
	 */
	public int getValue()
	{
		try
		{
			return (editText == null) ? 0 : Integer.parseInt(editText.getText()
					.toString());
		} catch (NumberFormatException e)
		{
			e.printStackTrace();
			return 0;
		}
	}
	
	/**
	 * 设置控件的值
	 * @param value 需要设置的值
	 */
	public void setValue(int value)
	{
		if (editText != null)
		{
			editText.setText(value + "");
		}
	}
	
	/**
	 * 设置滑动条的最大值
	 * @param max 需要设置的最大值
	 */
	public void setMax(int max)
	{
		seekBar.setMax(max);
	}
}

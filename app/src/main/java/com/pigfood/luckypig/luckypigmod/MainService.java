package com.pigfood.luckypig.luckypigmod;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.IBinder;
import android.provider.Settings;
import android.support.design.widget.TabLayout;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.TableLayout;
import android.widget.Toast;

public class MainService extends Service {
    private WindowManager window;
    private WindowManager.LayoutParams btnParams;
    private IconHolder iconViewHolder;
    private View imageView;

    @Override
    public void onCreate() {
        super.onCreate();
        window = (WindowManager) getApplication().getSystemService(WINDOW_SERVICE);
        iconViewHolder = new IconHolder();
        imageView = LayoutInflater.from(this).inflate(R.layout.icon, null);
        iconViewHolder.setImgBtn((ImageButton) imageView.findViewById(R.id.icon));
        iconViewHolder.getImgBtn().setOnTouchListener(new View.OnTouchListener() {
            boolean isMoved;
            float oldX, oldY;

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        isMoved = false;
                        oldX = event.getRawX();
                        oldY = event.getRawY();
                        break;
                    case MotionEvent.ACTION_MOVE:
                        if (Math.abs(event.getRawX() - oldX) > 2f && Math.abs(event.getRawY() - oldY) > 2f) {
                            isMoved = true;
                            btnParams.x = (int) event.getRawX() - imageView.getMeasuredWidth() / 2;
                            btnParams.y = (int) event.getRawY() - imageView.getMeasuredHeight();
                            window.updateViewLayout(imageView, btnParams);
                        }
                        break;
                    case MotionEvent.ACTION_UP:
                        if (!isMoved) {
                            Intent i = new Intent(MainService.this, PageActivity.class);
                            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                            startActivity(i);
                            MainService.this.stopSelf();
                        }
                        break;
                }
                return true;
            }
        });
        //set btnParams
        btnParams = new WindowManager.LayoutParams();
        btnParams.type = WindowManager.LayoutParams.TYPE_PHONE;
        btnParams.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
        btnParams.format = PixelFormat.RGBA_8888;
        btnParams.gravity = Gravity.TOP | Gravity.LEFT;
        btnParams.width = WindowManager.LayoutParams.WRAP_CONTENT;
        btnParams.height = WindowManager.LayoutParams.WRAP_CONTENT;
        if (Build.VERSION.SDK_INT >= 23 && !Settings.canDrawOverlays(getApplicationContext())) {
            stopSelf();
            return;
        }
        window.addView(imageView, btnParams);
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        window.removeView(imageView);
    }

    class IconHolder {
        private ImageButton imgBtn;

        public ImageButton getImgBtn() {
            return imgBtn;
        }

        public void setImgBtn(ImageButton imgBtn) {
            this.imgBtn = imgBtn;
        }
    }
}

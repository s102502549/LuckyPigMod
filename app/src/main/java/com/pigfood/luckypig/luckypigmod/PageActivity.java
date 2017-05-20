package com.pigfood.luckypig.luckypigmod;

import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import android.support.design.widget.TabLayout;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Toast;

import static android.provider.Settings.ACTION_MANAGE_OVERLAY_PERMISSION;

public class PageActivity extends AppCompatActivity {
    private MainHolder mainViewHolder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_page);
        mainViewHolder = new MainHolder();
        mainViewHolder.setTab((TabLayout) findViewById(R.id.tab));
        mainViewHolder.setPager((ViewPager) findViewById(R.id.pager));
        mainViewHolder.getPager().setAdapter(new MainPagerAdapter(getSupportFragmentManager()));
        mainViewHolder.getTab().addOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
                if (tab.getPosition() == 1) {
                    PageActivity.this.finish();
                    return;
                }
                mainViewHolder.getPager().setCurrentItem(tab.getPosition());
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {

            }

            @Override
            public void onTabReselected(TabLayout.Tab tab) {

            }
        });
        if (Build.VERSION.SDK_INT >= 23 && !Settings.canDrawOverlays(getApplicationContext())) {
            Intent i = new Intent(ACTION_MANAGE_OVERLAY_PERMISSION, Uri.parse("package:" + getPackageName()));
            Toast.makeText(this, R.string.open_top, Toast.LENGTH_SHORT).show();
            startActivity(i);
        }
    }

    @Override
    public void onBackPressed() {
        //super.onBackPressed();
        moveTaskToBack(true);
        startService(new Intent(this, MainService.class));
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    class MainHolder {
        private TabLayout tab;
        private ViewPager pager;

        public TabLayout getTab() {
            return tab;
        }

        public void setTab(TabLayout tab) {
            this.tab = tab;
        }

        public ViewPager getPager() {
            return pager;
        }

        public void setPager(ViewPager pager) {
            this.pager = pager;
        }
    }

    class MainPagerAdapter extends FragmentPagerAdapter {

        public MainPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        @Override
        public Fragment getItem(int position) {
            if (position == 0)
                return SearchFragment.newInstance();
            else
                return null;
        }

        @Override
        public int getCount() {
            return 1;
        }
    }
}

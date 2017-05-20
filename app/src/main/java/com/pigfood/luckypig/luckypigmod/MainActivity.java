package com.pigfood.luckypig.luckypigmod;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;

public class MainActivity extends Activity {
    private MainHolder viewHolder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        viewHolder = new MainHolder();
        viewHolder.setInfo((TextView) findViewById(R.id.info));
        viewHolder.setExit((Button) findViewById(R.id.exit_button));
        viewHolder.setStart((Button) findViewById(R.id.start_button));
        File f = new File(getFilesDir().toString() + "/read");
        File f1 = new File(getFilesDir().toString() + "/write");
        File f2 = new File(getFilesDir().toString() + "/list");
        File f3 = new File(getFilesDir().toString() + "/display");
        if (!(f.exists() && f1.exists() && f2.exists() && f3.exists()))
            new ExtractTask().execute();
    }


    public void onStartClicked(View view) {
        startActivity(new Intent(this, PageActivity.class));
        finish();
    }

    public void onExitClicked(View view) {
        finish();
    }

    private class MainHolder {
        private TextView info;
        private Button exit;
        private Button start;

        public TextView getInfo() {
            return info;
        }

        public void setInfo(TextView info) {
            this.info = info;
        }

        public Button getExit() {
            return exit;
        }

        public void setExit(Button exit) {
            this.exit = exit;
        }

        public Button getStart() {
            return start;
        }

        public void setStart(Button start) {
            this.start = start;
        }
    }

    private class ExtractTask extends AsyncTask<Void, Void, Boolean> {

        private ProgressDialog progress;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            progress = ProgressDialog.show(MainActivity.this, "安裝中", "正在為第一次啟動做準備");
        }

        @Override
        protected Boolean doInBackground(Void... params) {
            try {
                BufferedInputStream readInput;
                BufferedInputStream writeInput;
                BufferedOutputStream readOutput = new BufferedOutputStream(MainActivity.this.openFileOutput("read", Context.MODE_PRIVATE));
                BufferedOutputStream writeOutput = new BufferedOutputStream(MainActivity.this.openFileOutput("write", Context.MODE_PRIVATE));
                String abi = Build.CPU_ABI;
                if (abi.contains("armeabi")) {
                    readInput = new BufferedInputStream(MainActivity.this.getAssets().open("armeabi/read"));
                    writeInput = new BufferedInputStream(MainActivity.this.getAssets().open("armeabi/write"));
                } else if (abi.contains("x86")) {
                    readInput = new BufferedInputStream(MainActivity.this.getAssets().open("x86/read"));
                    writeInput = new BufferedInputStream(MainActivity.this.getAssets().open("x86/write"));
                } else
                    return false;
                int readNum;
                byte[] buffer = new byte[4096];
                //copy read
                while ((readNum = readInput.read(buffer)) > 0)
                    readOutput.write(buffer, 0, readNum);
                //copy write
                while ((readNum = writeInput.read(buffer)) > 0)
                    writeOutput.write(buffer, 0, readNum);
                //close stream
                readInput.close();
                readOutput.close();
                writeInput.close();
                writeOutput.close();
                //set executable
                new File(MainActivity.this.getFilesDir() + "/read").setExecutable(true);
                new File(MainActivity.this.getFilesDir() + "/write").setExecutable(true);
                //create needed file
                MainActivity.this.openFileOutput("list", Context.MODE_PRIVATE).close();
                MainActivity.this.openFileOutput("display", Context.MODE_PRIVATE).close();
                return true;
            } catch (IOException e) {
                e.printStackTrace();
                return false;
            }
        }

        @Override
        protected void onPostExecute(Boolean isSuccess) {
            super.onPostExecute(isSuccess);
            progress.dismiss();
            if (!isSuccess) {
                viewHolder.getInfo().setText(R.string.cannot_use);
                viewHolder.getStart().setClickable(false);
            }
        }
    }
}

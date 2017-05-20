package com.pigfood.luckypig.luckypigmod;

import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import org.w3c.dom.ls.LSInput;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;


public class SearchFragment extends Fragment {
    private SearchViewHolder viewHolder;
    private boolean isFirstSearch;
    private HashMap<String, String> types;
    private HashMap<String, String> ops;
    private int pid;
    private String address;

    public SearchFragment() {
        // Required empty public constructor
    }


    public static SearchFragment newInstance() {
        SearchFragment fragment = new SearchFragment();
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        viewHolder = new SearchViewHolder();
        isFirstSearch = true;
        types = new HashMap<>();
        ops = new HashMap<>();
        types.put("byte", "b");
        types.put("word", "s");
        types.put("dword", "i");
        types.put("qword", "li");
        types.put("float", "f");
        types.put("double", "d");
        ops.put("=", "e");
        ops.put("<", "l");
        ops.put(">", "b");
        ops.put("change", "c");
        ops.put("unchange", "u");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_search, container, false);
        prepareHolder(v);
        addListener();
        return v;
    }

    public void prepareHolder(View v) {
        viewHolder.setText_pids((TextView) v.findViewById(R.id.pids));
        viewHolder.setBtn_reSearch((Button) v.findViewById(R.id.re_search_btn));
        viewHolder.setSpin_type((Spinner) v.findViewById(R.id.type));
        viewHolder.setSpin_ops((Spinner) v.findViewById(R.id.spin_ops));
        viewHolder.setText_value((EditText) v.findViewById(R.id.value));
        viewHolder.setBtn_search((Button) v.findViewById(R.id.btn_search));
        viewHolder.setBtn_1((Button) v.findViewById(R.id.btn_1));
        viewHolder.setBtn_2((Button) v.findViewById(R.id.btn_2));
        viewHolder.setBtn_3((Button) v.findViewById(R.id.btn_3));
        viewHolder.setBtn_4((Button) v.findViewById(R.id.btn_4));
        viewHolder.setBtn_5((Button) v.findViewById(R.id.btn_5));
        viewHolder.setBtn_6((Button) v.findViewById(R.id.btn_6));
        viewHolder.setBtn_7((Button) v.findViewById(R.id.btn_7));
        viewHolder.setBtn_8((Button) v.findViewById(R.id.btn_8));
        viewHolder.setBtn_9((Button) v.findViewById(R.id.btn_9));
        viewHolder.setBtn_0((Button) v.findViewById(R.id.btn_0));
        viewHolder.setBtn_dot((Button) v.findViewById(R.id.btn_dot));
        viewHolder.setBtn_back((Button) v.findViewById(R.id.btn_back));
        viewHolder.setList((ListView) v.findViewById(R.id.list_result));
        viewHolder.setText_warn((TextView) v.findViewById(R.id.text_warn));
    }

    public void addListener() {
        View.OnClickListener onNumClick = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                viewHolder.getText_value().append(((Button) v).getText());
            }
        };
        viewHolder.getBtn_1().setOnClickListener(onNumClick);
        viewHolder.getBtn_2().setOnClickListener(onNumClick);
        viewHolder.getBtn_3().setOnClickListener(onNumClick);
        viewHolder.getBtn_4().setOnClickListener(onNumClick);
        viewHolder.getBtn_5().setOnClickListener(onNumClick);
        viewHolder.getBtn_6().setOnClickListener(onNumClick);
        viewHolder.getBtn_7().setOnClickListener(onNumClick);
        viewHolder.getBtn_8().setOnClickListener(onNumClick);
        viewHolder.getBtn_9().setOnClickListener(onNumClick);
        viewHolder.getBtn_0().setOnClickListener(onNumClick);
        viewHolder.getBtn_dot().setOnClickListener(onNumClick);
        viewHolder.getBtn_back().setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int length = viewHolder.getText_value().getText().length();
                if (length > 0) {
                    viewHolder.getText_value().getText().delete(length - 1, length);
                }
            }
        });
        viewHolder.getText_pids().setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (isFirstSearch) {
                    ActivityManager am = (ActivityManager) getContext().getSystemService(Context.ACTIVITY_SERVICE);
                    final List<ActivityManager.RunningAppProcessInfo> list = am.getRunningAppProcesses();
                    new AlertDialog.Builder(getContext()).setAdapter(new PidsAdapter(list), new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            viewHolder.getText_pids().setText(list.get(which).pid + " : " + list.get(which).processName);
                            pid = list.get(which).pid;
                            Log.e("pid", "" + pid);
                        }
                    }).show();
                }
            }
        });
        viewHolder.getBtn_search().setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new SearchTask(pid, types.get(viewHolder.getSpin_type().getSelectedItem().toString()), ops.get(viewHolder.getSpin_ops().getSelectedItem().toString()), viewHolder.getText_value().getText().toString()).execute();
            }
        });
        viewHolder.getList().setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                address = viewHolder.getList().getItemAtPosition(position).toString();
                View v = LayoutInflater.from(getContext()).inflate(R.layout.set_value, null);
                final EditText text = (EditText) v.findViewById(R.id.value);
                new AlertDialog.Builder(getContext()).setView(v).setPositiveButton("確定", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        try {
                            Log.e("com", "su -c " + getContext().getFilesDir() + "/write " + pid + " " + types.get(viewHolder.getSpin_type().getSelectedItem().toString()) + " " + text.getText().toString() + " " + address.split(" ")[0]);
                            Process p = Runtime.getRuntime().exec("su -c " + getContext().getFilesDir() + "/write " + pid + " " + types.get(viewHolder.getSpin_type().getSelectedItem().toString()) + " " + text.getText().toString() + " " + address.split(" ")[0]);
                            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(p.getOutputStream()));
                            writer.write("exit\n");
                            writer.close();
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }).show();
            }
        });
        viewHolder.getBtn_reSearch().setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                isFirstSearch = true;
                viewHolder.getText_warn().setText("");
                viewHolder.getList().setAdapter(null);
            }
        });
    }

    class SearchViewHolder {
        private TextView text_pids;
        private Button btn_reSearch;
        private Spinner spin_type;
        private Spinner spin_ops;
        private EditText text_value;
        private Button btn_search;
        private Button btn_1;
        private Button btn_2;
        private Button btn_3;
        private Button btn_4;
        private Button btn_5;
        private Button btn_6;
        private Button btn_7;
        private Button btn_8;
        private Button btn_9;
        private Button btn_0;
        private Button btn_dot;
        private Button btn_back;
        private ListView list;
        private TextView text_warn;

        public Spinner getSpin_type() {
            return spin_type;
        }

        public void setSpin_type(Spinner spin_type) {
            this.spin_type = spin_type;
        }

        public EditText getText_value() {
            return text_value;
        }

        public void setText_value(EditText text_value) {
            this.text_value = text_value;
        }

        public ListView getList() {
            return list;
        }

        public void setList(ListView list) {
            this.list = list;
        }

        public TextView getText_warn() {
            return text_warn;
        }

        public void setText_warn(TextView text_warn) {
            this.text_warn = text_warn;
        }

        public TextView getText_pids() {
            return text_pids;
        }

        public void setText_pids(TextView text_pids) {
            this.text_pids = text_pids;
        }

        public Button getBtn_reSearch() {
            return btn_reSearch;
        }

        public void setBtn_reSearch(Button btn_reSearch) {
            this.btn_reSearch = btn_reSearch;
        }

        public Button getBtn_search() {
            return btn_search;
        }

        public void setBtn_search(Button btn_search) {
            this.btn_search = btn_search;
        }

        public Button getBtn_1() {
            return btn_1;
        }

        public void setBtn_1(Button btn_1) {
            this.btn_1 = btn_1;
        }

        public Button getBtn_2() {
            return btn_2;
        }

        public void setBtn_2(Button btn_2) {
            this.btn_2 = btn_2;
        }

        public Button getBtn_3() {
            return btn_3;
        }

        public void setBtn_3(Button btn_3) {
            this.btn_3 = btn_3;
        }

        public Button getBtn_4() {
            return btn_4;
        }

        public void setBtn_4(Button btn_4) {
            this.btn_4 = btn_4;
        }

        public Button getBtn_5() {
            return btn_5;
        }

        public void setBtn_5(Button btn_5) {
            this.btn_5 = btn_5;
        }

        public Button getBtn_6() {
            return btn_6;
        }

        public void setBtn_6(Button btn_6) {
            this.btn_6 = btn_6;
        }

        public Button getBtn_7() {
            return btn_7;
        }

        public void setBtn_7(Button btn_7) {
            this.btn_7 = btn_7;
        }

        public Button getBtn_8() {
            return btn_8;
        }

        public void setBtn_8(Button btn_8) {
            this.btn_8 = btn_8;
        }

        public Button getBtn_9() {
            return btn_9;
        }

        public void setBtn_9(Button btn_9) {
            this.btn_9 = btn_9;
        }

        public Button getBtn_0() {
            return btn_0;
        }

        public void setBtn_0(Button btn_0) {
            this.btn_0 = btn_0;
        }

        public Button getBtn_dot() {
            return btn_dot;
        }

        public void setBtn_dot(Button btn_dot) {
            this.btn_dot = btn_dot;
        }

        public Button getBtn_back() {
            return btn_back;
        }

        public void setBtn_back(Button btn_back) {
            this.btn_back = btn_back;
        }

        public Spinner getSpin_ops() {
            return spin_ops;
        }

        public void setSpin_ops(Spinner spin_ops) {
            this.spin_ops = spin_ops;
        }
    }

    class PidsAdapter extends BaseAdapter {
        private List<ActivityManager.RunningAppProcessInfo> processList;

        PidsAdapter(List<ActivityManager.RunningAppProcessInfo> processList) {
            this.processList = processList;
        }

        @Override
        public int getCount() {
            return processList.size();
        }

        @Override
        public Object getItem(int position) {
            return processList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null)
                convertView = LayoutInflater.from(getContext()).inflate(android.R.layout.simple_list_item_1, null);
            TextView text = (TextView) convertView.findViewById(android.R.id.text1);
            text.setText(processList.get(position).pid + " : " + processList.get(position).processName);
            return convertView;
        }
    }

    class SearchTask extends AsyncTask<Void, Integer, Void> {
        private ProgressDialog progress;
        private int pid;
        private String type;
        private String op;
        private String value;

        public SearchTask(int pid, String type, String op, String value) {
            this.pid = pid;
            this.type = type;
            this.op = op;
            this.value = value;
        }

        @Override
        protected Void doInBackground(Void... params) {
            Process p;
            try {
                if (isFirstSearch) {
                    if (value.equals("")) {
                        p = Runtime.getRuntime().exec("su -c " + getContext().getFilesDir() + "/read " + pid + " n " + type + " n " + 1);
                        Log.e("com", "su -c " + getContext().getFilesDir() + "/read " + pid + " n " + type + " n " + 1);
                    } else {
                        p = Runtime.getRuntime().exec("su -c " + getContext().getFilesDir() + "/read " + pid + " n " + type + " " + op + " " + value);
                        Log.e("com", "su -c " + getContext().getFilesDir() + "/read " + pid + " n " + type + " " + op + " " + value);
                    }
                } else {
                    p = Runtime.getRuntime().exec("su -c " + getContext().getFilesDir() + "/read " + pid + " s " + type + " " + op + " " + value);
                    Log.e("com", "su -c " + getContext().getFilesDir() + "/read " + pid + " s " + type + " " + op + " " + value);
                }
                String msg;
                BufferedReader reader = new BufferedReader(new InputStreamReader(p.getInputStream()));
                BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(p.getOutputStream()));
                while ((msg = reader.readLine()) != null) {
                    Log.e("aaa", msg);
                    if (msg.equals("finish"))
                        break;
                    else if (msg.contains("stopped"))
                        writer.write("fg\n");
                }
                writer.write("exit\n");
                reader.close();
                writer.close();

            } catch (IOException e) {
                e.printStackTrace();
            }
            return null;
        }

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            progress = ProgressDialog.show(getContext(), "搜尋中", "請稍後");
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            super.onPostExecute(aVoid);
            progress.dismiss();
            isFirstSearch = false;
            try {
                BufferedReader reader = new BufferedReader(new InputStreamReader(getContext().openFileInput("display")));
                ArrayList<String> list = new ArrayList<>();
                String s;
                while ((s = reader.readLine()) != null) {
                    Log.e("msg", s);
                    list.add(s);
                }
                viewHolder.getList().setAdapter(new ArrayAdapter<String>(getContext(), android.R.layout.simple_list_item_1, list));
                if (list.isEmpty())
                    viewHolder.getText_warn().setText("找不到數據，請重新搜尋");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

    }

}

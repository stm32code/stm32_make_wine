package com.example.intelligentwarehousestorage

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.ArrayAdapter
import android.widget.SeekBar
import com.blankj.utilcode.util.LogUtils
import com.example.intelligentwarehousestorage.bean.DataDTO
import com.example.intelligentwarehousestorage.bean.Receive
import com.example.intelligentwarehousestorage.bean.Send
import com.example.intelligentwarehousestorage.databinding.ActivityMainBinding
import com.example.intelligentwarehousestorage.utils.BeatingAnimation
import com.google.gson.Gson
import com.gyf.immersionbar.ImmersionBar
import com.itfitness.mqttlibrary.MQTTHelper
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken
import org.eclipse.paho.client.mqttv3.MqttCallback
import org.eclipse.paho.client.mqttv3.MqttMessage
import com.example.intelligentwarehousestorage.utils.Common
import com.example.intelligentwarehousestorage.utils.Common.PushTopic
import com.example.intelligentwarehousestorage.utils.MToast
import com.example.intelligentwarehousestorage.utils.TimeCycle

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private var isDebugView = false //是否显示debug界面
    private var arrayList = ArrayList<String>()// debug消息列表
    private var adapter: ArrayAdapter<String>? = null// debug适配器
    private var onlineFlag = false //是否在线标识
    private var Waring1 = false
    private var Waring2= false
    private var Waring3 = false
    private var Waring4 = false
    private var Waring5 = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        mqttServer()
        initView()
        isOnline()
    }

    /**
     * 界面的初始化
     */
    private fun initView() {
        setSupportActionBar(binding.toolbar)
        ImmersionBar.with(this).init()
        debugView()
        warringLayout(false, "")
        eventManager()


    }

    private fun eventManager() {
     
 


    /**
     * @brief debug界面的初始化
     */
    private fun debugView() {
        adapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, arrayList)
        binding.debugView.adapter = adapter
    }

    /**
     * mqtt服务
     */
    private fun mqttServer() {
        Common.mqttHelper = MQTTHelper(
            this, Common.Sever, Common.DriveID, Common.DriveName, Common.DrivePassword, true, 30, 60
        )
        Common.mqttHelper!!.connect(Common.ReceiveTopic, 1, true, object : MqttCallback {
            override fun connectionLost(cause: Throwable?) {
                MToast.mToast(this@MainActivity, "连接已经断开")
            }

            override fun messageArrived(topic: String?, message: MqttMessage?) {
                //收到消息
                val data = Gson().fromJson(message.toString(), Receive::class.java)
                LogUtils.eTag("接收到消息", message?.payload?.let { String(it) })
                onlineFlag = true
                binding.online.text = "在线"
                debugViewData(2, message?.payload?.let { String(it) }!!)
                println(data)
                if (data != null) {
                    analysisOfData(data)
                } else {
                    MToast.mToast(this@MainActivity, "数据异常")
                }
            }

            override fun deliveryComplete(token: IMqttDeliveryToken?) {

            }
        })

    }

    /***
     * 数据处理
     */
    private fun analysisOfData(data: Receive) {
        try {

            if (data.temp != null) {
                binding.temperatureText.text = String.format("%s", data.temp)
                // 已触发
                // Waring1 = data.temp!!.toFloat() > 28F && data.temp!!.toFloat() < 30F

            }
            if (data.water != null) {
                if (data.water.equals("0")) {
                    binding.humidityText.text = "充足"
                    Waring2 = false
                } else {
                    binding.humidityText.text = "缺水"
                    Waring2 = true

                }


            }
 


        } catch (e: Exception) {
            e.printStackTrace()
            MToast.mToast(this, "数据解析失败:$e")
        }
    }

    /***
     * 判断硬件是否在线
     */
    private fun isOnline() {
        Thread {
            var i = 0
            while (true) {//开启轮询，每18s重置标识
                if (i > 3) {
                    i = 0
                    runOnUiThread {
                        binding.online.text = if (onlineFlag) "在线" else "离线"
                    }
                    onlineFlag = false
                }
                i++
                Thread.sleep(20000)
            }
        }.start()
    }
    /**
     * @brief 再次封装MQTT发送
     * @param message 需要发送的消息
     */
    private fun sendMessage(cmd: Int, message: String) {
        if (Common.mqttHelper != null && Common.mqttHelper!!.subscription) {
            var str = ""
            when (cmd) {
                3 -> {
                    str = Gson().toJson(
                        Send(
                            cmd = 3,
                            ph_v = binding.CO2SeekBar.progress,
                            mq_3_v = binding.mq3SeekBar.progress
                        )
                    )
                }

            }
            Thread {
                Common.mqttHelper!!.publish(
                    PushTopic, str, 1
                )
            }.start()

            debugViewData(1, str)
        }
    }
    /**
     * @brief debug界面数据添加
     * @param str 如果为 1 添加发送数据到界面   为 2 添加接受消息到界面
     */
    private fun debugViewData(str: Int, data: String) {
        if (arrayList.size >= 255) arrayList.clear()
        runOnUiThread {
            when (str) {
                1 -> { //发送的消息
                    arrayList.add("目标主题:${Common.ReceiveTopic} \n时间:${TimeCycle.getDateTime()}\n发送消息:$data ")
                }

                2 -> {
                    arrayList.add("来自主题:${Common.ReceiveTopic} \n时间:${TimeCycle.getDateTime()}\n接到消息:$data ")
                }
            }
            // 在添加新数据之后调用以下方法，滚动到列表底部
            binding.debugView.post {
                binding.debugView.setSelection(adapter?.count?.minus(1)!!)
            }
            adapter?.notifyDataSetChanged()
        }

    }

    /**
     * @brief 显示警告弹窗和设置弹窗内容
     * @param visibility 是否显示
     * @param str 显示内容
     */
    private fun warringLayout(visibility: Boolean, str: String) {
        if (visibility) {
            binding.warringLayout.visibility = View.VISIBLE
            binding.warringText.text = str
            BeatingAnimation().onAnimation(binding.warringImage)
        } else {
            binding.warringLayout.visibility = View.GONE
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_scrolling, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        return when (item.itemId) {
            R.id.setDebugView -> { // set debug view is enabled or disabled
                isDebugView = !isDebugView
                binding.debugView.visibility = if (isDebugView) View.VISIBLE else View.GONE
                true
            }

            else -> super.onOptionsItemSelected(item)
        }
    }

}
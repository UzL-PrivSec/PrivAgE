package clientsim

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.lang.Thread.sleep
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonArray
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.put
import kotlinx.serialization.json.buildJsonObject

import clientsim.HttpUtils

class DummySummation(name: String) {

    private val IP = "http://server:8888/dummysum/$name/"

    private var stepDuration = -1L
    private var id = ""

    /*
     * Protocol Interface
     */


    private suspend fun start(x: DoubleArray) {

        step0()

        withContext(Dispatchers.IO) {
            sleep(stepDuration)
        }

        step1(x)
    }

    suspend fun waitAndStart(x: DoubleArray): Boolean {
        val MAX_NUM_TRIES = 20
        var tries = 0
        var status = HttpUtils.GET("${IP}ready").jsonObject["status"]!!.jsonPrimitive!!.content
        while (status != "READY" && tries < MAX_NUM_TRIES) {
            println(status)
            withContext(Dispatchers.IO) {
                sleep(2500)
            }
            status = HttpUtils.GET("${IP}ready").jsonObject["status"]!!.jsonPrimitive!!.content
            tries += 1
        }
        if (status == "READY") {
            withContext(Dispatchers.IO) {
                sleep(2000)
            }
            start(x)
        }
        return status == "READY"
    }

    suspend fun waitAndGetResult(): DoubleArray  {
        val MAX_NUM_TRIES = 20
        var tries = 0
        var status = HttpUtils.GET("${IP}result").jsonObject["status"]!!.jsonPrimitive!!.content
        while (status.equals("ongoing") && tries < MAX_NUM_TRIES) {
            withContext(Dispatchers.IO) {
                sleep(2500)
            }
            status = HttpUtils.GET("${IP}result").jsonObject["status"]!!.jsonPrimitive!!.content
            tries += 1
        }
        val result = HttpUtils.GET("${IP}result").jsonObject["result"]!!.jsonArray
        return result.map { it.jsonPrimitive.content.toDouble() }.toMutableList().toDoubleArray()
    }


    /*
     *  DummySum Protocol
     */


    private suspend fun step0() {
        val response = HttpUtils.POST("${IP}0")

        id = response.jsonObject["id"].toString()

        stepDuration = response.jsonObject["step_duration"].toString().toLong() * 1000 // secs to millis

        println(id)
    }


    private suspend fun step1(x: DoubleArray) {

        val jsonObject = buildJsonObject {
            put("id", id)
            put("vector", x.joinToString(";"))
        }

        HttpUtils.POST("${IP}1", jsonObject)
    }
}
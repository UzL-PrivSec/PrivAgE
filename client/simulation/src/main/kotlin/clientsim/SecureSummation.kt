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
import kotlin.math.pow

import clientsim.HttpUtils


class SecureSummation(name: String) {

    private val IP = "http://server:8888/secsum/$name/"

    // public protocol params
    private var t = ""
    private var l = ""
    private var step_duration = -1L
    private var precision = 0

    // local crypto variables
    private var id = ""
    private var mySecretKey1 = ""
    private var mySecretKey2 = ""
    private var myPubKey1 = ""
    private var myPubKey2 = ""
    private var myNeighbours: MutableList<String> = mutableListOf<String>()
    private val pk1Map = HashMap<String, String>()
    private val pk2Map = HashMap<String, String>()
    private var bBase64 = ""
    private val ngcijMap = HashMap<String, String>()
    private val avNgsA1 = mutableListOf<String>()
    private var R1 = mutableListOf<String>()
    private var R2 = mutableListOf<String>()

    init {
        System.loadLibrary("secsum")
    }


    /*
     * Protocol Interface
     */


    private suspend fun start(x: DoubleArray) {

        step0()

        withContext(Dispatchers.IO) {
            sleep(step_duration)
        }

        step1()

        withContext(Dispatchers.IO) {
            sleep(step_duration)
        }

        step21()


        withContext(Dispatchers.IO) {
            sleep(step_duration)
        }

        step22()

        // Until now, there were no dropouts allowed.
        // Potential Dropouts
        // Here, dropouts will have the effect that clients are not part of the final sum.

        withContext(Dispatchers.IO) {
            sleep(step_duration)
        }

        step3()

        // Potential Dropouts
        // Here, dropouts will have the effect that clients are not part of the final sum.

        withContext(Dispatchers.IO) {
            sleep(step_duration)
        }

        step4()

        // Potential Dropouts
        // Here, dropouts will have the effect that clients are not part of the final sum.

        withContext(Dispatchers.IO) {
            sleep(step_duration)
        }

        step5(x)

        // Potential Dropouts
        // Here, inputs of dropouts have to be reconstructed

        withContext(Dispatchers.IO) {
            sleep(step_duration)
        }

        step6()

        // Potential Dropouts
        // Here, inputs of dropouts have to be reconstructed

        withContext(Dispatchers.IO) {
            sleep(step_duration)
        }

        step7()
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
     *  SecSum Protocol
     */


    private suspend fun step0() {
        val response = HttpUtils.POST("${IP}0")

        id = response.jsonObject["id"].toString()

        step_duration = response.jsonObject["step_duration"].toString().toLong() * 1000 // secs to millis

        precision = response.jsonObject["precision"].toString().toInt()

        println(id)
    }


    private suspend fun step1() {

        val jsonObject = buildJsonObject {
            put("id", id)
        }

        val response = HttpUtils.POST("${IP}1", jsonObject)

        t = response.jsonObject["t"].toString()
        l = response.jsonObject["l"].toString()
        myNeighbours =
            response.jsonObject["neighbours"]!!.jsonArray.map { it.jsonPrimitive.content }
                .toMutableList()

        val data = Step2Data()
        JNIstep2(data)

        mySecretKey1 = data.privKey1
        mySecretKey2 = data.privKey2
        myPubKey1 = data.pubKey1
        myPubKey2 = data.pubKey2
    }


    private suspend fun step21() {

        val jsonObject = buildJsonObject {
            put("id", id)
            put("pubKey1", myPubKey1)
            put("pubKey2", myPubKey2)
        }

        HttpUtils.POST("${IP}2", jsonObject)
    }


    private suspend fun step22() {

        val jsonObject = buildJsonObject {
            put("id", id)
        }

        val response = HttpUtils.POST("${IP}3", jsonObject)

        for (s in response.jsonObject["key_dict"]!!.jsonArray) {
            val tempList = s.jsonPrimitive.content.split(";").toMutableList()
            val client = tempList[0]
            pk1Map[client] = tempList[1]
            pk2Map[client] = tempList[2]
        }
    }


    private suspend fun step3() {

        val data = Step3Data()

        JNIstep3(
            t.toFloat().toInt(),
            mySecretKey1,
            mySecretKey2,
            id,
            pk2Map,
            data
        )

        bBase64 = data.bBase64

        val jsonMapObject = buildJsonObject {
            data.cipherMap.forEach { (key, value) ->
                put(key, value)
            }
        }
        val jsonObject = buildJsonObject {
            put("id", id)
            put("cijMap", jsonMapObject)
        }

        HttpUtils.POST("${IP}4", jsonObject)
    }


    private suspend fun step4() {

        val jsonObject = buildJsonObject {
            put("id", id)
        }

        val response = HttpUtils.POST("${IP}5", jsonObject)

        for (s in response.jsonObject["cij_list"]!!.jsonArray) {
            val tempList = s.jsonArray
            val ng = tempList[0].jsonPrimitive.content
            val cij = tempList[1].jsonPrimitive.content
            ngcijMap[ng] = cij
        }

        for (ng in myNeighbours) {
            if (ngcijMap.keys.contains(ng)) {
                avNgsA1.add(ng)
            } else {
                pk1Map.remove(ng)
                pk2Map.remove(ng)
            }
        }

        println("Dropped NGs: " + (myNeighbours.size - avNgsA1.size))
    }


    private suspend fun step5(x: DoubleArray) {

        val xInt = x.map { (it * 10.toDouble().pow(precision)).toInt() }.toIntArray()

        val data = Step5Data()

        JNIstep5(
            xInt,
            l.toInt(),
            avNgsA1.toList(),
            pk1Map,
            mySecretKey1,
            id,
            bBase64,
            data
        )

        val jsonObject = buildJsonObject {
            put("id", id)
            put("masked_input", data.maskedInput)
        }

        HttpUtils.POST("${IP}6", jsonObject)

    }


    private suspend fun step6() {

        val jsonObject = buildJsonObject {
            put("id", id)
        }

        val response = HttpUtils.POST("${IP}7", jsonObject)

        R1 = response.jsonObject["R1"]!!.jsonArray.map { it.toString() }.toMutableList()
        R2 = response.jsonObject["R2"]!!.jsonArray.map { it.toString() }.toMutableList()
    }


    private suspend fun step7() {

        val data = Step7Data()

        JNIstep7(id, mySecretKey2, ngcijMap, pk2Map, data)

        val hbs = HashMap<String, String>();
        val hss = HashMap<String, String>();

        for (decCipher in data.decodedCiphers) {
            val splitDecCipher = decCipher.split(";")
            val i = splitDecCipher[0]
            hbs[i] = splitDecCipher[2]
            hss[i] = splitDecCipher[3]
        }

        val jsonHbsObject = buildJsonObject {
            hbs.forEach { (key, value) ->
                if (key in R1) {
                    put(key, value)
                }
            }
        }
        val jsonHssObject = buildJsonObject {
            hss.forEach { (key, value) ->
                if (key in R2) {
                    put(key, value)
                }
            }
        }
        val jsonObject = buildJsonObject {
            put("id", id)
            put("hbs", jsonHbsObject);
            put("hss", jsonHssObject)
        }

        HttpUtils.POST("${IP}8", jsonObject)
    }


    /*
     * JNI
     */


    data class Step2Data(
        val pubKey1: String = "",
        val pubKey2: String = "",
        val privKey1: String = "",
        val privKey2: String = ""
    )

    private external fun JNIstep2(data: Step2Data): Void


    data class Step3Data(
        val bBase64: String = "",
        val cipherMap: HashMap<String, String> = HashMap()
    )

    private external fun JNIstep3(
        threshold: Int,
        base64SecKey1: String,
        base64SecKey2: String,
        clientId: String,
        pk2Map: HashMap<String, String>,
        data: Step3Data
    ): Void


    data class Step5Data(
        val maskedInput: String = ""
    )

    private external fun JNIstep5(
        x: IntArray,
        l: Int,
        a1: List<String>,
        pk1Map: HashMap<String, String>,
        secKey1Base64: String,
        id: String,
        bBase64: String,
        data: Step5Data
    ): Void


    data class Step7Data(
        val decodedCiphers: MutableList<String> = mutableListOf()
    )

    private external fun JNIstep7(
        id: String,
        secKey2Base64: String,
        encodedMsgsMap: HashMap<String, String>,
        pk2Map: HashMap<String, String>,
        data: Step7Data
    ): Void
}
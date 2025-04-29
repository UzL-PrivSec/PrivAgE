package clientsim

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonObject
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.OutputStream
import java.net.HttpURLConnection
import java.net.URL


object HttpUtils {

    suspend fun sendHttpRequest(
        urlString: String,
        method: String,
        body: JsonObject? = null
    ): JsonElement = withContext(Dispatchers.IO) {
        val url = URL(urlString)
        val connection = url.openConnection() as HttpURLConnection
        connection.requestMethod = method
        connection.setRequestProperty("Content-Type", "application/json; charset=UTF-8")

        if (method == "POST" && body != null) {
            connection.doOutput = true
            connection.outputStream.use { outputStream ->
                outputStream.write(body.toString().toByteArray(Charsets.UTF_8))
            }
        }

        val responseCode = connection.responseCode
        val responseString = if (responseCode == HttpURLConnection.HTTP_OK) {
            connection.inputStream.bufferedReader().use(BufferedReader::readText)
        } else {
            connection.errorStream?.bufferedReader()?.use(BufferedReader::readText)
                ?: throw Exception("HTTP error $responseCode with no details.")
        }

        connection.disconnect()
        Json.parseToJsonElement(responseString)
    }


    suspend fun GET(urlString: String): JsonElement =
        sendHttpRequest(urlString, "GET")


    suspend fun POST(urlString: String, body: JsonObject? = null): JsonElement =
        sendHttpRequest(urlString, "POST", body)

}
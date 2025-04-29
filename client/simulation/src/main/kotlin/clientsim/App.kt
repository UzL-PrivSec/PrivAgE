package clientsim

suspend fun main() {

    val x = DoubleArray(16) { i -> i * 0.1 }

    for (i in 0..1) {
        // val sumProtocol = SecureSummation("Iteration-$i")
        val sumProtocol = DummySummation("Iteration-$i")
        val flag = sumProtocol.waitAndStart(x)
        if (!flag) {
            return
        }
        val result = sumProtocol.waitAndGetResult()
        println(result.joinToString(" "))
    } 
}
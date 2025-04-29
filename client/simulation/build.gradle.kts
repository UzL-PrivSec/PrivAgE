plugins {
	kotlin("jvm") version "1.9.20"
	kotlin("plugin.serialization") version "1.9.20"
	
	application
}

repositories {
	mavenCentral()
}

dependencies {
	implementation(kotlin("stdlib-common"))
	implementation("io.ktor:ktor-client-core:2.3.5")
	implementation("io.ktor:ktor-client-cio:2.3.5")
	implementation("io.ktor:ktor-client-json:2.3.5")
	implementation("io.ktor:ktor-client-content-negotiation:2.3.5")
	implementation("io.ktor:ktor-serialization-kotlinx-json:2.3.5")
	implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.6.0")
}

application {
	mainClass.set("clientsim.AppKt")
}
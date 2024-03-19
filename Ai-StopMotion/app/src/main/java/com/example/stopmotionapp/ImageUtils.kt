@file:Suppress("SpellCheckingInspection")
package com.example.stopmotionapp

import android.app.Activity
import android.content.ContentResolver
import android.content.ContentValues
import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.net.Uri
import android.os.Environment
import android.provider.MediaStore
import android.widget.Toast
import com.arthenica.ffmpegkit.FFmpegKit
import org.opencv.android.Utils
import org.opencv.core.Core
import org.opencv.core.Mat
import org.opencv.imgproc.Imgproc
import java.io.File
import java.io.FileOutputStream

private var WIDTH: Int = 16
private var HEIGHT: Int = 16

/*
Sisältää kuvien käsittelyyn liittyviä funktioita
*/

// Muuttaa listan kuvia videoksi ffmpeg-kirjaston avulla.
fun convertUrisToVideo(context: Context, uris: List<Uri>, frameRate: Int) {

    (context as Activity).runOnUiThread {
        Toast.makeText(context, "Creating your video...", Toast.LENGTH_SHORT).show()
    }

    val outputDirectory = context.getExternalFilesDir(Environment.DIRECTORY_DCIM)
    val outputVideoFile = File(outputDirectory, "output.mp4")
    // Luo väliaikaiset tiedostot
    val tempFiles = uris.map { File.createTempFile("tmp",".jpg") }

    // Kopioi tiedot väliaikaisiin tiedostoihin
    val resolver = context.contentResolver
    for ((index, uri) in uris.withIndex()) {
        val inputStream = resolver.openInputStream(uri)
        inputStream?.use { input ->
            FileOutputStream(tempFiles[index]).use { output ->
                input.copyTo(output)
            }
        }
    }
    // Kirjaa tekstitiedostoon tarvittavat tiedot (kuvien polut ja jokaisen
    // kuvan esittämisjan)
    val file = File(context.cacheDir, "list.txt")
    file.createNewFile()
    file.writeText("")

    for (temp in tempFiles) {
        val canonicalPath = temp.canonicalPath
        val duration = 1.0 / frameRate
        file.appendText("file '$canonicalPath'\n")
        file.appendText("duration $duration\n")
    }


    // Rakentaa ffmpeg-komennon
    val command = arrayOf(
        "-y",
        "-f", "concat",
        "-safe", "0",
        "-i", file.absolutePath,
        "-c:v", "mpeg4",
        "-vf", "scale=iw/2:ih/2",
        "-preset", "ultrafast",
        "-pix_fmt", "yuv420p",
        outputVideoFile.path
    )

    val ffmpegCommand = command.joinToString(" ")

    // Pyörittää komennon
    FFmpegKit.executeAsync(ffmpegCommand) { session ->
        val returnCode = session.returnCode
        if (returnCode.isValueSuccess) {
            println("FFmpeg process exited successfully")
            saveMediaToGallery(context, outputVideoFile,false)

            context.runOnUiThread {
                Toast.makeText(context, "Your video is done!", Toast.LENGTH_SHORT).show()
            }
        } else {
            println("FFmpeg process exited with error: $returnCode")
        }

        // Poistaa väliaikaiset tiedostot
        tempFiles.forEach { it.delete() }
    }
}

// Tallentaa median galleriaan.
fun saveMediaToGallery(context: Context, mediaFile: File, isImage: Boolean) {

    val contentValues = ContentValues().apply {
        put(MediaStore.MediaColumns.DISPLAY_NAME,
            if (isImage){"image_"} else {"video_"} + System.currentTimeMillis() + if (isImage) {".jpg"} else {".mp4"})
        put(MediaStore.MediaColumns.DATE_ADDED, System.currentTimeMillis() / 1000)
        put(MediaStore.MediaColumns.DATE_MODIFIED, System.currentTimeMillis() / 1000)
        put(MediaStore.MediaColumns.RELATIVE_PATH, "DCIM/AIStopMotion")
        put(MediaStore.MediaColumns.MIME_TYPE,
            if (isImage) {"image/jpeg"} else {"video/mp4"})
    }
    val resolver = context.contentResolver
    val uri = resolver.insert(
        if (isImage) MediaStore.Images.Media.EXTERNAL_CONTENT_URI else MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
        contentValues)
    uri?.let {
        resolver.openOutputStream(it)?.use { outputStream ->
            mediaFile.inputStream().use { inputStream ->
                inputStream.copyTo(outputStream)
            }
        }
    }
}

// Laskee kahden bitmapin samankaltaisuuden OpenCV-kirjaston avulla.
fun calculateSimilarity(image1: Bitmap?, image2: Bitmap?): Double {

    // Skaalaus
    val scaled1 = scaleBitmap(image1, WIDTH, HEIGHT)
    val scaled2 = scaleBitmap(image2, WIDTH, HEIGHT)

    // Muutetaan Mat-olioiksi
    val mat1 = Mat()
    val mat2 = Mat()
    Utils.bitmapToMat(scaled1, mat1)
    Utils.bitmapToMat(scaled2, mat2)


    // Haetaan reunat
    val edges1 = Mat()
    val edges2 = Mat()
    Imgproc.Canny(mat1, edges1, 400.0, 100.0)
    Imgproc.Canny(mat2, edges2, 400.0, 100.0)

    // Verrataan reunoja
    val diff = Mat()
    Core.absdiff(edges1, edges2, diff)
    val sqrDiff = Mat()
    Core.multiply(diff, diff, sqrDiff)

    // Lasketaan vertailuluku
    val meanSqrDiff = Core.mean(sqrDiff)
    val similarity = 1.0 / (1.0 + meanSqrDiff.`val`[0])

    // Palautetaan vertailuku
    return similarity * 100

}

// Skaalaa bitmapin halutun kokoiseksi.
fun scaleBitmap(bitmap: Bitmap?, width: Int, height: Int): Bitmap? {
    return bitmap?.let {
        Bitmap.createScaledBitmap(it, width, height, false)
    }
}


// Palauttaa bitmapin Urin pohjalta.
fun getBitmapFromUri(contentResolver: ContentResolver, uri: Uri?): Bitmap? {
    return try {
        val inputStream = uri?.let { contentResolver.openInputStream(it) }
        BitmapFactory.decodeStream(inputStream)
    } catch (e: Exception) {
        e.printStackTrace()
        null
    }
}


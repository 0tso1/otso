@file:Suppress("SpellCheckingInspection")

package com.example.stopmotionapp

import androidx.appcompat.app.AppCompatActivity
import com.example.stopmotionapp.databinding.ActivityMainBinding
import android.Manifest
import android.app.Activity
import android.content.Intent
import android.content.SharedPreferences
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.graphics.Matrix
import android.media.MediaPlayer
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import android.widget.Toast
import android.util.Log
import android.view.View.*
import androidx.activity.result.contract.ActivityResultContracts
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageCapture
import androidx.camera.core.ImageCaptureException
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.net.toFile
import androidx.exifinterface.media.ExifInterface
import androidx.preference.PreferenceManager
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.opencv.android.OpenCVLoader
import java.io.File
import java.text.SimpleDateFormat
import java.util.*

/*
Tämä AI-StopMotion sovellus luo stop motion animaatioita ottamalla automaattisesti kuvia näkymää
muutettaessa ja koostamalla otetuista kuvista videon.

Sovellus ottaa kuvia säädetyn intervallin välein ja vertaa otettua kuvaa tunnettuun referenssi-
kuvaan. Mikäli kuva on riittävän erilainen (esim. käsi siirtää kohteita) tallennetaan seuraavaa kuva
jonka vastaavasti tulee olla riittävän samankaltainen tunnetun referenssikuvan kanssa. Tallenetut
kuvat ilmestyvät skaalattuina RecyclerView-karuselliin, sekä imageList:lle. Tallennus-painikkeella
käyttäjä saa koostettua kuvista (imageList) videon, joka tallenetaan galleriaan.

Sovelluksessa on lisäksi asetusnäkymä, joka käyttää Preferences-kirjastoa tallentaakseen ja esittääkseen
käyttäjän asettamat asetukset.
*/
class MainActivity : AppCompatActivity() {

    // Intervalli kuvien ottamiselle (ms)
    val INTERVAL: Long = 500

    // Aktiviteetin päälayoutin binding-luokka.
    private lateinit var viewBinding: ActivityMainBinding
    // Kameran käyttämä Executor-palvelu.
    private lateinit var cameraExecutor: ExecutorService

    // Kameran kuvausobjekti.
    private var imageCapture: ImageCapture? = null
    // Viimeisimmän tallennetun kuvan Uri.
    var savedUri: Uri? = null
    // Tila, joka ilmaisee onko ohjelma valmis ottamaan uutta kuvaa.
    private var readyForNewPic: Boolean = false

    // Päivityspalvelun Handler-olio.
    private lateinit var mainHandler: Handler
    // RecyclerView tallennetuista kuvista.
    private lateinit var newRecyclerView: RecyclerView


    // Kuvan vertailuun ja tallennukseen käytettävä Runnable-olio.
    private val assessPhotoRunnable = object : Runnable {
        override fun run() {
            myScope.launch { assessPhoto() }
            mainHandler.postDelayed(this, INTERVAL) // Otetaan kuva joka INTERVALLin välein
        }
    }

    // Tallennettujen kuvien Uri-lista.
    private val imageList: MutableList<Uri> = mutableListOf()
    // CoroutineScope-olio, joka ajaa assessPhoto-metodia pääsäikeessä.
    private val myScope = CoroutineScope(Dispatchers.Main)

    // Asetusten käsittelyä varten
    private lateinit var sharedPreferences: SharedPreferences
    private lateinit var settingsFragment: SettingsFragment

    // Äänen toistamista varten
    private var mMediaPlayer: MediaPlayer? = null

    //RecyclerView:n skaalattujen kuvien lista
    private var recyclerViewList: MutableList<Bitmap> = mutableListOf()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewBinding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(viewBinding.root)
        supportActionBar?.hide() //Piilottaa yläpalkin
        cameraExecutor = Executors.newSingleThreadExecutor()

        // Käsittelee kamera-napin painalluksen
        viewBinding.imageCaptureButton.setOnClickListener {
            myScope.launch { handleImageCaptureClick()}
        }
        viewBinding.addImagesButton.setOnClickListener{
            openGalleryForImages()
        }

        // Alustaa OpenCV-kirjaston
        if (!OpenCVLoader.initDebug()) {
            Toast.makeText(this, "Failed to load OpenCV", Toast.LENGTH_SHORT).show()
        }

        // Pyytää kameran luvat
        if (allPermissionsGranted()) {
            startCamera()
        } else {
            ActivityCompat.requestPermissions(
                this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS
            )
        }

        // Loopperi joka pyörittää kameraa
        mainHandler = Handler(Looper.getMainLooper())


        // RecyclerViewin asettaminen ja alustaminen
        newRecyclerView = viewBinding.recyclerView
        newRecyclerView.layoutManager = LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false)
        newRecyclerView.setHasFixedSize(true)

        viewBinding.createVideoButton.setOnClickListener{
            // Kutsuu videonrakentajaa
            if(!mainHandler.hasCallbacks(assessPhotoRunnable)  && imageList.isNotEmpty()){
                val fps = sharedPreferences.getString("video_fps", "15")!!.toInt()
                convertUrisToVideo(this,imageList,fps)
                val size = imageList.size
                imageList.clear()
                recyclerViewList.clear()
                newRecyclerView.adapter?.notifyItemRangeRemoved(0,size)
            }
            else{
                Toast.makeText(this, "No images or recording in progress.", Toast.LENGTH_SHORT).show()
            }
        }
        // Asetukset
        sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this)
        settingsFragment = SettingsFragment()

        // Asetus-nappi
        viewBinding.settingsButton.setOnClickListener {
            toggleSettingsFragment()
        }
    }

    // Avaa ja sulkee asetusikkunan eli SettingsFragment-luokan riippuen sen tilasta.
    private fun toggleSettingsFragment() {

        if (settingsFragment.isVisible) {
            viewBinding.settingsButton.text = getString(R.string.gear_emoji)
            supportFragmentManager.beginTransaction()
                .setCustomAnimations(android.R.anim.fade_in, android.R.anim.fade_out)
                .hide(settingsFragment)
                .commit()

        } else {
            viewBinding.settingsButton.text = getString(R.string.cross_emoji)
            supportFragmentManager.beginTransaction()
                .setCustomAnimations(android.R.anim.fade_in, android.R.anim.fade_out)
                .replace(R.id.settings_container, settingsFragment)
                .show(settingsFragment)
                .commit()

        }
    }


    // Otttaa ja äsittelee otetun kuvan. Tekee päätöksen tallettaa tai hylätä kuva.
    private suspend fun assessPhoto() {
        takePhoto {}

        // Haetaan asetukset
        val settings = arrayOf(
            sharedPreferences.getString("change_threshold", "3.0")!!.toDouble(),
            sharedPreferences.getString("image_threshold", "3.0")!!.toDouble())

        // Tuorein otettu kuva
        val latestUri = imageList.lastOrNull()

        val similarity = withContext(myScope.coroutineContext) {
            val bitmapNewest = getBitmapFromUri(contentResolver,latestUri) // Vertailukuva
            val bitmapPrevious = getBitmapFromUri(contentResolver, savedUri)
            calculateSimilarity(bitmapNewest, bitmapPrevious) // Laskee samankaltaisuuden
        }

        // Ensin halutaan, että kuva muuttuu radikaalisti, esimerkiksi käden tullessa kuvaan mukaan
        if(similarity <= settings[0] && !readyForNewPic){
            readyForNewPic = true
        }

        // Sitten odotetaan, että kuvaa palaa takaisin kohti vertailukuvan asettamaa "normaalia"
        else if(similarity >= settings[1] && readyForNewPic){
            saveImageToGallery(savedUri){
                readyForNewPic = false
                playSound() //Ääni kertoo kuvan talletuksesta
            }
        }
    }

    // Käsittellee kamera-napin painalluksen mekaniikan
    private fun handleImageCaptureClick() {


        // Ottaa ensimmäisen referenssikuvan ja lisää sen listaan,
        // sekä käynnistää pienellä viiveellä ajastminen.
        if (!mainHandler.hasCallbacks(assessPhotoRunnable)) {
            takePhoto {
                saveImageToGallery(savedUri) {
                    mainHandler.post(assessPhotoRunnable)
                    viewBinding.imageCaptureButton.text = getString(R.string.recording_emoji)
                    playSound()
                }
            }
        }

        // Sammuttaa ajastminen
        else{
            mainHandler.removeCallbacks(assessPhotoRunnable)
            viewBinding.imageCaptureButton.text = getString(R.string.camera_emoji) // Vaihtaa ikonin takaisin
        }
    }

    // Pyytää tarvitut luvat.
    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<String>, grantResults:
        IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == REQUEST_CODE_PERMISSIONS) {
            if (allPermissionsGranted()) {
                startCamera()
            } else {
                Toast.makeText(this,
                    "Permissions not granted by the user.",
                    Toast.LENGTH_SHORT).show()
                finish()
            }
        }
    }

    // Palataa output-kansion väliaikaisille tiedostoille.
    private fun getOutputDirectory(): File {
        val appContext = applicationContext
        val mediaDir = appContext.externalCacheDir?.let {
            File(it, "temp").apply { mkdirs() }
        }
        return if ((mediaDir != null) && mediaDir.exists())
            mediaDir else appContext.filesDir
    }

    // Ottaa kuvan kameralla.
    private fun takePhoto(callback: () -> Unit) {
        // Luo tilapäinen tiedoston kuvan tallentamista varten.
        val outputDirectory = getOutputDirectory()
        val name = SimpleDateFormat(FILENAME_FORMAT, Locale.FRENCH)
            .format(System.currentTimeMillis())
        val tempFile = File.createTempFile(name, ".jpg", outputDirectory)

        // Määritä tulostusasetukset, jotka sisältävät tiedoston ja sen metatiedot.
        val outputOptions = ImageCapture.OutputFileOptions.Builder(tempFile).build()

        // Aseta kuuntelijan, jota kutsutaan kuvan ottamisen jälkeen.
        imageCapture?.takePicture(
            outputOptions,
            ContextCompat.getMainExecutor(this),
            object : ImageCapture.OnImageSavedCallback {
                override fun onError(exc: ImageCaptureException) {
                }
                override fun onImageSaved(output: ImageCapture.OutputFileResults) {
                    savedUri = output.savedUri ?: Uri.fromFile(tempFile)
                    callback()
                }
            }
        )
    }

    // Tallentaa kuvan galleriaan SaveMediaToGallery-funktion avulla. Pävittää
    // RecyclerView:n.
    private fun saveImageToGallery(uri: Uri?, callback: () -> Unit) {
        if (uri != null) {
            imageList.add(uri)
            handleRecyclerView(uri)
        }
        uri?.let {
            saveMediaToGallery(this, it.toFile(), true)
            Toast.makeText(this, "Image saved to Gallery", Toast.LENGTH_SHORT).show()
            callback()
        }
    }

    // Käynniistää kameran ja asettaa viewFinderin.
    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(this)

        cameraProviderFuture.addListener({
            // Käytetään kameroiden elinkaaren sitomiseen elinkaaren omistajaan
            val cameraProvider: ProcessCameraProvider = cameraProviderFuture.get()

            // Asettaa preview:n
            val preview = Preview.Builder()
                .build()
                .also {
                    it.setSurfaceProvider(viewBinding.viewFinder.surfaceProvider)
                }

            imageCapture = ImageCapture.Builder().build()

            // Valitsee takakameran
            val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA

            try {
                // Irrota sidonnaisuuden ennen uudelleen sitomista
                cameraProvider.unbindAll()

                // Sitoo käyttötapauksen kameraan
                cameraProvider.bindToLifecycle(
                    this, cameraSelector, preview, imageCapture)

            } catch(exc: Exception) {
                Log.e(TAG, "Use case binding failed", exc)
            }

        }, ContextCompat.getMainExecutor(this))

    }

    // Tarkistaa mikäli pyydetyt luvat annettu.
    private fun allPermissionsGranted() = REQUIRED_PERMISSIONS.all {
        ContextCompat.checkSelfPermission(
            baseContext, it) == PackageManager.PERMISSION_GRANTED
    }

    // Avaa gallerian kuvia varten.
    private fun openGalleryForImages() {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT)
        intent.type = "image/*"
        intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true)
        resultLauncher.launch(intent)
    }

    // Alustaa result_launcherin toiminnolle, jonka avulla käyttäjä voivat valita kuvia
    private val resultLauncher = registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->

        if (result.resultCode == Activity.RESULT_OK) {
            val clipData = result.data?.clipData
            if (clipData != null) {
                for (i in 0 until clipData.itemCount) {
                    val imageUri = clipData.getItemAt(i).uri
                    imageList.add(imageUri)
                    handleRecyclerView(imageUri)
                }
            } else {
                val imageUri = result.data?.data
                if (imageUri != null) {
                    imageList.add(imageUri)
                    handleRecyclerView(imageUri)
                }
            }
        }
    }

    // Lisää skaalatut kuvat recyclerView:n ja skrollaa tämän kohdilleen.
    // Mikäli kuvat lisättäisiin suoraan imageList:lta veisi tämä kohtuuttomasti
    // resursseja.
    private fun handleRecyclerView(imageUri: Uri){

        val bmp = getBitmapFromUri(contentResolver, imageUri)

        // Kääntää bitmapin oikein
        val inputStream = contentResolver.openInputStream(imageUri)
        val exif = inputStream?.let { ExifInterface(it) }
        val orientation = exif?.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_UNDEFINED)
        val matrix = Matrix()

        when (orientation) {
            ExifInterface.ORIENTATION_ROTATE_90 -> matrix.postRotate(90f)
            ExifInterface.ORIENTATION_ROTATE_180 -> matrix.postRotate(180f)
            ExifInterface.ORIENTATION_ROTATE_270 -> matrix.postRotate(270f)
        }
        val rotatedBmp = bmp?.let { Bitmap.createBitmap(it, 0, 0, it.width, it.height, matrix, true) }

        // Skaalaa bitmapin
        val h = rotatedBmp?.height?.div(10)
        val w = rotatedBmp?.width?.div(10)
        val bmpScaled = scaleBitmap(rotatedBmp, w ?: 50, h ?: 50)

        if (bmpScaled != null) {
            recyclerViewList.add(bmpScaled)
        }

        // Lisää bitmapin recyclerView:iin
        newRecyclerView.adapter = CustomAdapter(recyclerViewList)
        newRecyclerView.adapter?.notifyItemInserted(recyclerViewList.size-1)
        newRecyclerView.adapter?.let { adapter ->
            if (adapter.itemCount > 0) {
                newRecyclerView.smoothScrollToPosition(adapter.itemCount - 1)
            }
        }
        inputStream?.close()
    }

    // onDestroy: tyhjää ajastimen ja sulkee kameran
    override fun onDestroy() {
        super.onDestroy()
        mainHandler.removeCallbacks(assessPhotoRunnable)
        cameraExecutor.shutdown()
    }

    // companion object: Sisältää joitain tärkeitä tietoja
    companion object {
        private const val TAG = "AIStopMotion"
        private const val FILENAME_FORMAT = "yyyy-MM-dd-HH-mm-ss-SSS"
        private const val REQUEST_CODE_PERMISSIONS = 10
        private val REQUIRED_PERMISSIONS =
            mutableListOf (
                Manifest.permission.CAMERA
            ).toTypedArray()
    }

    // Soittaa klikkaus-äänen
    private fun playSound() {
        if (mMediaPlayer == null) {
            mMediaPlayer = MediaPlayer.create(this, R.raw.click)
            mMediaPlayer!!.start()
        } else mMediaPlayer!!.start()
    }

    // onPause: Piilottaa asetusikkunan ja sulkee kameran
    override fun onPause() {
        super.onPause()
        cameraExecutor.shutdown()
    }

    // onStop: tuhoaa mediasoittimen ja piilottaa asetusikkunan
    override fun onStop() {
        super.onStop()
        if (mMediaPlayer != null) {
            mMediaPlayer!!.release()
            mMediaPlayer = null
        }
    }
}

@file:Suppress("SpellCheckingInspection")
package com.example.stopmotionapp

import android.graphics.Bitmap
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import androidx.recyclerview.widget.RecyclerView



class CustomAdapter(private var list: MutableList<Bitmap> = mutableListOf()) :
    RecyclerView.Adapter<CustomAdapter.ViewHolder>() {

    // ViewHolder luokka
    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val imageView: ImageView
        init {
            // ImageView alustus
            imageView = view.findViewById(R.id.iv_image)
        }
    }

    // Luo uudet View:t
    override fun onCreateViewHolder(viewGroup: ViewGroup, viewType: Int): ViewHolder {
        // Luo uuden View:N
        val view = LayoutInflater.from(viewGroup.context)
            .inflate(R.layout.recycler_view_card, viewGroup, false)

        return ViewHolder(view)
    }

    // Vaihtaa View:n sisällöt
    override fun onBindViewHolder(viewHolder: ViewHolder, position: Int) {
        // Asettaa bitmapit esille
        viewHolder.imageView.setImageBitmap(list[position])

    }

    // Palauttaa listan koon
    override fun getItemCount() = list.size

}
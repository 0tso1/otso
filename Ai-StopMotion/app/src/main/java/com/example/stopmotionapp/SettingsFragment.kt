package com.example.stopmotionapp

import android.os.Bundle
import androidx.preference.PreferenceFragmentCompat

/*
SettingsFragment-luokka, joka laajentaa PreferenceFragmentCompat-luokan.
Vastuussa käyttäjän muokattavissa olevien asetusten luettelon näyttämisestä.

Asetusten määrittely ja asettelu löytyy preferences.xml-tiedostosta.

*/
class SettingsFragment : PreferenceFragmentCompat() {

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        setPreferencesFromResource(R.xml.preferences, rootKey)
        context?.theme?.applyStyle(R.style.PreferenceScreen, true)
    }

}

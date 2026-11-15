/*
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.settings;

import android.content.Intent;
import android.os.Bundle;

import androidx.annotation.DrawableRes;
import androidx.annotation.StringRes;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragment;
import androidx.preference.PreferenceScreen;

import org.lineageos.settings.charge.ChargeControlActivity;
import org.lineageos.settings.refreshrate.RefreshActivity;
import org.lineageos.settings.saturation.SaturationActivity;
import org.lineageos.settings.speaker.ClearSpeakerActivity;

public class DashboardFragment extends PreferenceFragment {
    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        PreferenceScreen screen = getPreferenceManager().createPreferenceScreen(getContext());
        setPreferenceScreen(screen);

        addEntry(screen, R.string.refresh_title, R.string.refresh_summary,
                R.drawable.ic_qs_refresh_rate, RefreshActivity.class);
        addEntry(screen, R.string.saturation_title, R.string.saturation_summary,
                R.drawable.ic_saturation_tile, SaturationActivity.class);
        addEntry(screen, R.string.clear_speaker_title, R.string.clear_speaker_summary,
                R.drawable.ic_clear_speaker, ClearSpeakerActivity.class);
        addEntry(screen, R.string.charge_control_title, R.string.charge_control_summary,
                R.drawable.ic_bypass_charging, ChargeControlActivity.class);
    }

    private void addEntry(PreferenceScreen screen, @StringRes int title,
            @StringRes int summary, @DrawableRes int icon, Class<?> activityClass) {
        Preference preference = new Preference(getContext());
        preference.setTitle(title);
        preference.setSummary(summary);
        preference.setIcon(icon);
        preference.setOnPreferenceClickListener(item -> {
            startActivity(new Intent(getContext(), activityClass));
            return true;
        });
        screen.addPreference(preference);
    }
}

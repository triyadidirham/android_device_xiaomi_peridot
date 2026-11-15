/*
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.settings;

import android.os.Bundle;

import com.android.settingslib.collapsingtoolbar.CollapsingToolbarBaseActivity;

public class DashboardActivity extends CollapsingToolbarBaseActivity {
    private static final String TAG_DASHBOARD = "xiaomi_parts_dashboard";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setTitle(R.string.app_title);
        getFragmentManager().beginTransaction().replace(
                com.android.settingslib.collapsingtoolbar.R.id.content_frame,
                new DashboardFragment(), TAG_DASHBOARD).commit();
    }
}

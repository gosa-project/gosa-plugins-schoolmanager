<input type="hidden" name="phase_00">

<br><h3>{t 1="1/11"}STEP %1: Choose archive type{/t}</h3>
<p>
    {t}Choose archive type. teacher and students are seperate. The students home directory will be deleted directly.{/t}
    {t}The teachers home directory will be removed after a period of time.{/t}
</p>
<table summary="{t}Choose archive type{/t}">
	<tr>
		<td style="vertical-align: middle;">
			<br />
			<LABEL for="archivetype_id">{t}Type of user to archive{/t}</LABEL>
			<br />
            {html_radios name="archivetype_id" options=$available_archive_types selected=$preset_archive_type_id separator='<br />'}
		</td>
	</tr>
</table>

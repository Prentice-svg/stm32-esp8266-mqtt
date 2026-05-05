<template>
	<view class="section-card">
		<view v-if="title || subtitle" class="section-head">
			<view>
				<text v-if="title" class="section-title">{{ title }}</text>
				<text v-if="subtitle" class="section-subtitle">{{ subtitle }}</text>
			</view>
			<slot name="action"></slot>
		</view>
		<slot></slot>
	</view>
</template>

<script>
export default {
	name: 'SectionCard',
	props: {
		title: {
			type: String,
			default: ''
		},
		subtitle: {
			type: String,
			default: ''
		}
	}
}
</script>

<style>
.section-card {
	padding: 28rpx;
	border-radius: 32rpx;
	background: #ffffff;
	box-shadow: 0 16rpx 38rpx rgba(15, 23, 42, 0.065);
	border: 1rpx solid rgba(226, 232, 240, 0.72);
}

.section-head {
	margin-bottom: 20rpx;
	display: flex;
	align-items: center;
	justify-content: space-between;
	gap: 20rpx;
}

.section-title {
	display: block;
	font-size: 30rpx;
	font-weight: 900;
	color: #0F172A;
}

.section-subtitle {
	display: block;
	margin-top: 6rpx;
	font-size: 22rpx;
	color: #94A3B8;
}
</style>
